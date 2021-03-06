/*
 * MacGitver
 * Copyright (C) 2012-2015 Sascha Cunz <sascha@macgitver.org>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License (Version 2) as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QStringBuilder>

#include "libGitWrap/Result.hpp"
#include "libGitWrap/Index.hpp"
#include "libGitWrap/Remote.hpp"
#include "libGitWrap/Repository.hpp"
#include "libGitWrap/Reference.hpp"
#include "libGitWrap/Diff.hpp"
#include "libGitWrap/DiffList.hpp"
#include "libGitWrap/Object.hpp"
#include "libGitWrap/Tag.hpp"
#include "libGitWrap/Tree.hpp"
#include "libGitWrap/Blob.hpp"
#include "libGitWrap/Commit.hpp"
#include "libGitWrap/RevisionWalker.hpp"
#include "libGitWrap/Submodule.hpp"
#include "libGitWrap/BranchRef.hpp"
#include "libGitWrap/TagRef.hpp"
#include "libGitWrap/NoteRef.hpp"

#include "libGitWrap/Operations/CommitOperation.hpp"

#include "libGitWrap/Private/IndexPrivate.hpp"
#include "libGitWrap/Private/RemotePrivate.hpp"
#include "libGitWrap/Private/RepositoryPrivate.hpp"
#include "libGitWrap/Private/ReferencePrivate.hpp"
#include "libGitWrap/Private/DiffPrivate.hpp"
#include "libGitWrap/Private/ObjectPrivate.hpp"
#include "libGitWrap/Private/SubmodulePrivate.hpp"
#include "libGitWrap/Private/RevisionWalkerPrivate.hpp"

#include <QDir>

namespace Git
{

    namespace Internal
    {
        /**
         * @internal
         *
         * @ingroup     GitWrap
         *
         * @class       RepositoryPrivate
         *
         * @brief       Private implementation of the Repository class.
         */

        /**
         * @brief       Constructor
         *
         * @param[in]   repo    the libgit2 repository
         */
        RepositoryPrivate::RepositoryPrivate( git_repository* repo )
            : mRepo(repo)
            , mIndex(nullptr)
        {
        }

        RepositoryPrivate::~RepositoryPrivate()
        {
            Q_ASSERT( mRepo );

            if (openedFrom.isValid()) {
                Submodule::Private* smp = dataOf<Submodule>(openedFrom);
                Q_ASSERT(smp);
                smp->mSubRepo = nullptr;
            }

            // This assert may not look right in the first place, but it IS:
            // mIndex is of type IndexPrivate* and will get a value as soon as Repository::index()
            // is called for the first time. IndexPrivate is a RepoObject and as such it increases
            // the reference counter on the Repository object. In fact, this means the Repository
            // will never be deleted unless the Index has gone _before_ - When the Index is deleted
            // (due to the refCount dropping to zero) it will set mIndex to nullptr.
            //
            // I'm documenting this mainly for one reason: Last week I spent 3 hours trying to fix
            // a race-condition in libgit2, which - as I later understood - is not at all there
            // because outer constraints - like the above - prohibited the race to happen.
            Q_ASSERT( !mIndex );

            git_repository_free( mRepo );
        }

        /**
         * @internal
         * @brief           Load the HEAD reference and return it
         *
         * @param[in,out]   result  A result object; see @ref GitWrapErrorHandling
         *
         * This method deliberately doesn't use `git_repository_head`, because that will try to
         * resolve the symbolic link and _always_ return a OID based reference.
         */
        Reference RepositoryPrivate::getHead(Result& result) const
        {
            GW_CHECK_RESULT( result, Reference() );

            git_reference* refHead = NULL;
            result = git_reference_lookup(&refHead, mRepo, "HEAD");
            GW_CHECK_RESULT( result, Reference() );

            RepositoryPrivate* me = const_cast<RepositoryPrivate*>(this);
            return new Reference::Private(me, refHead);
        }

        static int statusHashCB( const char* fn, unsigned int status, void* rawSH )
        {
            #if 0
            qDebug( "%s - %s", qPrintable( QString::number( status, 2 ) ), fn );
            #endif

            Git::StatusHash* sh = (Git::StatusHash*) rawSH;
            sh->insert( GW_StringToQt( fn ), convertFileStatus( status ) );

            return GIT_OK;
        }

        struct cb_append_reference_data
        {
            Result&             result;
            RepositoryPrivate*  ptr;
            ReferenceList       refs;

            cb_append_reference_data( Result& _result, RepositoryPrivate* _ptr )
                : result( _result )
                , ptr( _ptr )
            {
            }
        };

        static int cb_append_reference( git_reference *reference, void *payload )
        {
            cb_append_reference_data *data = static_cast< cb_append_reference_data* >( payload );
            Q_ASSERT( data );

            Reference::Private* ref =
                    Reference::Private::createRefObject( data->result, data->ptr,
                                                         GW_StringToQt(git_reference_name(reference)),
                                                         reference);
            GW_CHECK_RESULT( data->result, data->result.errorCode() );
            data->refs.append(ref);

            return 0;
        }

    }

    GW_PRIVATE_IMPL(Repository, Base)

    /**
     * @ingroup     GitWrap
     *
     * @class       Repository
     *
     * @brief       Represents a Git repository.
     */

    /**
     * @brief       Create a new repository
     *
     * A new git repository will be created in @a path. The path pointed to by @a path must either
     * be empty or not exist at all.
     *
     * Capabilities of the current operating system and the file system at @a path will be checked
     * and the repository's config will be setup accordingly.
     *
     * @param[in]       path    The path where the new repository will be created.
     * @param[in]       bare    If `true`, a bare repository will be created. If `false` a working
     *                          tree will be setup.
     * @param[in,out]	result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return  If successful, a `Repository` object for the newly created repostiory will be
     *          returned. Otherwise an invalid `Repository` object will be returned and the Result
     *          object is filled with the error.
     *
     * @see	Repository::open()
     */
    Repository Repository::create( Result& result,
                                   const QString& path,
                                   bool bare )
    {
        GW_CHECK_RESULT( result, Repository() );

        git_repository* repo = nullptr;
        result = git_repository_init( &repo, GW_StringFromQt(path), bare );
        GW_CHECK_RESULT( result, Repository() );

        return new Private(repo);
    }

    /**
     * @brief Lookup a git repository by walking parent directories starting from startPath
     *
     * The lookup ends when the first repository is found or when reaching one of the @a ceilingDirs
     * directories.
     *
     * The method will automatically detect if the repository is bare (if there is a repository).
     *
     * @param[in] startPath
     * The base path where the lookup starts.
     *
     * @param[in] acrossFs
     * If `true`, then the lookup will not stop when a filesystem change is detected
     * while exploring parent directories.
     *
     * @param[in] ceilingDirs
     * A list of absolute paths (not symbolic links). The lookup will stop when one of these
     * paths is reached and no repository was found.
     *
     * @param[in,out] result
     * A result object; see @ref GitWrapErrorHandling
     *
     * @return the path of the found repository or an empty QString
     *
     * @see Repository::open(), Repository::create()
     */
    QString Repository::discover( Result& result,
                                  const QString& startPath,
                                  bool acrossFs,
                                  const QStringList& ceilingDirs )
    {
        GW_CHECK_RESULT( result, QString() );

        qDebug( ) << "DIsCOVER PATH" << startPath;

        git_buf repoPath = {0};
        QByteArray joinedCeilingDirs = GW_EncodeQString( ceilingDirs.join(QChar::fromLatin1(GIT_PATH_LIST_SEPARATOR)) );
        result = git_repository_discover( &repoPath, GW_StringFromQt(startPath),
                                          acrossFs, joinedCeilingDirs.constData() );

        QString resultPath;
        if ( result ) {
            resultPath = GW_StringToQt(repoPath.ptr);
        }

        git_buf_free( &repoPath );

        return resultPath;
    }

    /**
     * @brief       Open an existing repository
     *
     * Opens the repository at @a path. The repository may be bare or have a working tree.
     *
     * This method will not try to discover a repository, if there is no repository found at
     * @a path.
     *
     * @param[in]       path    The path of the repository to open.
     *
     * @param[in,out]   result  A result object; see @ref GitWrapErrorHandling
     *
     * @return  If successful, a `Repository` object for the opened repostiory will be returned.
     *          Otherwise an invalid `Repository` object will be returned and the Result object
     *          is filled with the error.
     *
     * @sa	Repository::discover(), Repository::create()
     */
    Repository Repository::open(Result& result, const QString& path)
    {
        git_libgit2_init();
        GW_CHECK_RESULT( result, Repository() );

        git_repository* repo = nullptr;

        result = git_repository_open( &repo, GW_StringFromQt( path ) );
        GW_CHECK_RESULT( result, Repository() );

        return new Private(repo);
    }

    /**
     * @brief           Open a independent instance of this repository
     *
     * @param[in,out]   result  A result object; see @ref GitWrapErrorHandling
     *
     * @return          A Repository object that refers to the same repository, but actually is
     *                  absolutely independant of this repository object.
     *
     * The repository is opened using the working directory path, not the .git path.
     */
    Repository Repository::reopen(Result& result) const
    {
        return open( result, workTreePath() );
    }

    /**
     * @brief       Check if the repository is bare
     *
     * @return      `true`, if the repository is bare and `false` if not.
     *
     * This method will return `true` if this Repository object is invalid. It will also set the
     * per-thread Result to "Invalid object".
     */
    bool Repository::isBare() const
    {
        GW_CD(Repository);
        if( d )
        {
            return git_repository_is_bare( d->mRepo );
        }
        else
        {
            return true;
        }
    }

    /**
     * @brief       Check whether the repository's HEAD is detached
     *
     * @return      @c true if this Repository is valid and its HEAD branch points to a commit
     *              rather than to another reference. @c false if the Repository is either invalid
     *              or its HEAD points to another reference.
     *
     */
    bool Repository::isHeadDetached() const
    {
        GW_CD(Repository);
        return d && git_repository_head_detached(d->mRepo);
    }

    /**
     * @brief       Check whether the repository's HEAD is detached
     *
     * @return      @c true if this Repository is valid and its HEAD branch points to branch that
     *              does not yet exist. @c false in any other case.
     *
     */
    bool Repository::isHeadUnborn() const
    {
        GW_CD(Repository);
        return d && git_repository_head_unborn(d->mRepo);
    }

    /**
     * @brief           Access the repository's index object
     *
     * @param[in,out]   result	A result object; see @ref GitWrapErrorHandling
     *
     * @return          The repository's index object or an invalid Index object, if either the
     *                  repository isBare() or an invalid repository object.
     *
     * If this repository object is invalid, the @a result object will be set to
     * "Invalid object".
     */
    Index Repository::index( Result& result )
    {
        GW_D_CHECKED(Repository, Index(), result);

        if( isBare() ) {
            return Index();
        }

        if (!d->mIndex) {
            git_index* index = nullptr;

            result = git_repository_index(&index, d->mRepo);

            if (!result) {
                return Index();
            }

            d->mIndex = new Index::Private(d, index);
        }

        return d->mIndex;
    }

    /**
     * @brief           get the status of a single file
     *
     * @param[in,out]   result      A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       fileName    A path (relative to the repository) for which to get the
     *                              current status.
     *
     * @return                      the current file status
     *
     *                  The file status is a combination of worktree, index and repository HEAD.
     */
    Git::StatusFlags Repository::status(Result &result, const QString &fileName) const
    {
        GW_CD_CHECKED(Repository, FileInvalidStatus, result);

        unsigned int status = GIT_STATUS_CURRENT;
        result = git_status_file( &status, d->mRepo, GW_StringFromQt(fileName) );
        GW_CHECK_RESULT( result, FileInvalidStatus );

        return Internal::convertFileStatus( status );
    }

    StatusHash Repository::status(Result &result) const
    {
        GW_CD_CHECKED(Repository, StatusHash(), result);

        git_status_options opt = GIT_STATUS_OPTIONS_INIT;

        opt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED
                  | GIT_STATUS_OPT_INCLUDE_IGNORED
                  | GIT_STATUS_OPT_INCLUDE_UNMODIFIED
                  | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

        StatusHash sh;
        result = git_status_foreach_ext( d->mRepo, &opt, &Internal::statusHashCB, (void*) &sh );
        GW_CHECK_RESULT( result, StatusHash() );

        return sh;
    }

    /**
     * @brief           List all references
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return  A QStringList with all references of this repository.
     *
     */
    QStringList Repository::allReferenceNames( Result& result )
    {
        GW_D_CHECKED(Repository, QStringList(), result);

        git_strarray arr;
        result = git_reference_list( &arr, d->mRepo );
        GW_CHECK_RESULT( result, QStringList() );

        return Internal::slFromStrArray( &arr );
    }

    ReferenceList Repository::allReferences(Result &result)
    {
        GW_D_CHECKED(Repository, ReferenceList(), result);
        Internal::cb_append_reference_data data( result, d );
        result = git_reference_foreach( d->mRepo,
                                        &Internal::cb_append_reference,
                                        &data );
        GW_CHECK_RESULT( result, ReferenceList() );

        return data.refs;
    }

    namespace Internal
    {

        struct cb_enum_resolvedrefs_data
        {
            Result*         result;
            ResolvedRefs    refs;
            git_repository* repo;
        };

        static int cb_enum_resolvedrefs( git_reference* ref, void* payload )
        {
            cb_enum_resolvedrefs_data* d = (cb_enum_resolvedrefs_data*) payload;

            git_oid oid;

            const char *refName = git_reference_name(ref);
            int rc = git_reference_name_to_id( &oid, d->repo, refName );

            d->result->setError( rc );
            if( rc < 0 )
            {
                return -1;
            }

            d->refs.insert( GW_StringToQt( refName ), ObjectId::fromRaw( oid.id ) );

            return 0;
        }

    }

    ResolvedRefs Repository::allResolvedRefs( Result& result )
    {
        GW_CD_CHECKED(Repository, ResolvedRefs(), result);

        Internal::cb_enum_resolvedrefs_data data;
        data.repo = d->mRepo;
        data.result = &result;

        Result tmp( git_reference_foreach( d->mRepo, &Internal::cb_enum_resolvedrefs, &data ) );

        if (tmp.errorCode() == GIT_EUSER) {
            // correct error is already in result.
            return ResolvedRefs();
        }

        if (!tmp) {
            result = tmp;
            return ResolvedRefs();
        }

        return data.refs;
    }

    QStringList Repository::allBranchNames( Result& result )
    {
        return branchNames( result, true, true );
    }

    QString Repository::currentBranch(Result &result)
    {
        Reference refHEAD = HEAD( result );
        GW_CHECK_RESULT( result, QString() );

        return refHEAD.shorthand();
    }

    QStringList Repository::branchNames(Result& result, bool local, bool remote)
    {
        GW_CD_CHECKED(Repository, QStringList(), result);

        QStringList sl;

        git_branch_iterator* it = nullptr;
        git_branch_t type;
        int types = 0;

        if (local) {
            types |= GIT_BRANCH_LOCAL;
        }

        if (remote) {
            types |= GIT_BRANCH_REMOTE;
        }

        result = git_branch_iterator_new(&it, d->mRepo, static_cast<git_branch_t>(types));
        if (!result) {
            return sl;
        }

        int err;
        git_reference* ref;
        while ((err = git_branch_next(&ref, &type, it)) == GITERR_NONE) {
            Q_ASSERT(ref);

            QString name = GW_StringToQt(git_reference_shorthand(ref));
            sl << name;

            git_reference_free(ref);
            ref = nullptr;
        }

        if (err != GIT_ITEROVER) {
            result = err;
            return QStringList();
        }

        return sl;
    }

    bool Repository::renameBranch(Result& result, const QString& oldName, const QString& newName, bool force)
    {
        GW_CD_CHECKED(Repository, false, result);

        git_reference* ref = nullptr;

        result = git_branch_lookup( &ref, d->mRepo, GW_StringFromQt(oldName),
                                    GIT_BRANCH_LOCAL );

        if( result.errorCode() == GITERR_REFERENCE )
        {
            result = git_branch_lookup( &ref, d->mRepo, GW_StringFromQt(oldName),
                                        GIT_BRANCH_REMOTE );
        }

        if( !result )
        {
            return false;
        }

        git_reference* refOut = nullptr;
        result = git_branch_move(&refOut, ref, GW_StringFromQt(newName), force); //MEH

        if( result )
        {
            git_reference_free( refOut );
        }

        git_reference_free( ref );

        return result;
    }

    QStringList Repository::allTagNames( Result& result )
    {
        GW_CD_CHECKED(Repository, QStringList(), result);

        git_strarray arr;
        result = git_tag_list( &arr, d->mRepo );

        if( !result )
        {
            return QStringList();
        }

        return Internal::slFromStrArray( &arr );
    }

    /**
     * @brief       Get the path to the repository's worktree.
     *
     * @return      the path to the repository's working directory
     *
     * @see         Repository::path()
     *
     * If the repository is bare, the returned path is empty.
     */
    QString Repository::workTreePath() const
    {
        GW_CD(Repository);
        if (!d) {
            return QString();
        }

        return GW_StringToQt( git_repository_workdir( d->mRepo ) );
    }

    /**
     * @brief       Get the path to the repository database.
     *
     * @return      the path to the repository folder
     *
     * @see         Repository::workdir()
     *
     * The path returned is the resolved absolute path to the repository folder.
     *
     * For example, if the repository is linked via a .git file, the path
     * returned will be resolved to the location of the .git folder.
     * .git folder. This is commonly used in submodules.
     *
     * For example, assume a common non-bare repository. The repository's
     * ".git" folder and worktree both live in "/repositories/my-repo".
     * The repository contains a submodule with a .git file in subfolder
     * "my-module-4711".
     *
     * This example results in the following pathes:
     * - Repository path: "/repositories/my-repo/.git"
     * - Submodule path : "/repositories/my-repo/.git/modules/my-module-4711"
     * .
     */
    QString Repository::path() const
    {
        GW_CD(Repository);
        if (!d) {
            return QString();
        }

        return GW_StringToQt( git_repository_path( d->mRepo ) );
    }

    /**
     * @brief   Get the name of the Repository
     *
     * The name of a repository is the directory where the repository is located. If the trailing
     * `.git` is present, it will be stripped.
     *
     * @return  Name of the repository
     */
    QString Repository::name() const
    {
        QString repoPath( isBare() ? path() : workTreePath() );
        if( repoPath.endsWith( QChar( L'/') ) )
        {
            repoPath = repoPath.left( repoPath.length() - 1 );
        }

        QFileInfo fi( repoPath );

        if( fi.suffix() == QStringLiteral( "git" ) )
        {
            return fi.completeBaseName();
        }

        return fi.fileName();
    }

    Reference Repository::HEAD( Result& result ) const
    {
        GW_CD_CHECKED(Repository, Reference(), result);

        git_reference* refHead = nullptr;
        result = git_repository_head( &refHead, d->mRepo );
        GW_CHECK_RESULT( result, Reference() );

        return new Reference::Private(const_cast<Private*>(d), refHead);
    }

    /**
     * @brief           Resolves HEAD to the target branch reference (aka "active branch").
     *
     *                  Note that the result is invalid for a detached HEAD.
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return          the BranchRef for the active branch or an invalid object in case of failure
     */
    BranchRef Repository::headBranch(Result& result) const
    {
        return HEAD(result).resolved(result).asBranch();
    }

    QString Repository::headBranchName(Result& result) const
    {
        GW_CD_CHECKED(Repository, QString(), result);

        Reference refHead = d->getHead(result);

        if (!result) {
            return QString();
        }

        if (refHead.type() != ReferenceSymbolic) {
            return QString();
        }

        return refHead.target();
    }

    Object Repository::lookup( Result& result, const ObjectId& id, ObjectType ot )
    {
        GW_D_CHECKED(Repository, Object(), result);

        git_object* obj = nullptr;
        git_otype gitObjType = Internal::objectType2git(ot);

        result = git_object_lookup(&obj, d->mRepo, Private::sha(id), gitObjType);
        GW_CHECK_RESULT( result, Object() );

        return Object::Private::create(d, obj);
    }

    Commit Repository::lookupCommit(Result& result, const ObjectId& id)
    {
        return lookup( result, id, otCommit ).asCommit();
    }

    Tree Repository::lookupTree(Result& result, const ObjectId& id)
    {
        return lookup( result, id, otTree ).asTree();
    }

    Blob Repository::lookupBlob(Result& result, const ObjectId& id)
    {
        return lookup( result, id, otBlob ).asBlob();
    }

    Tag Repository::lookupTag(Result& result, const ObjectId& id)
    {
        return lookup( result, id, otTag ).asTag();
    }

    Object Repository::lookup( Result& result, const QString& refName, ObjectType ot )
    {
        return lookup(result, reference(result, refName).resolveToObjectId(result), ot);
    }

    Commit Repository::lookupCommit(Result& result, const QString& refName)
    {
        return lookupCommit(result, reference(result, refName).resolveToObjectId(result));
    }

    Tree Repository::lookupTree(Result& result, const QString& refName)
    {
        return lookupTree(result, reference(result, refName).resolveToObjectId(result));
    }

    Blob Repository::lookupBlob(Result& result, const QString& refName)
    {
        return lookupBlob(result, reference(result, refName).resolveToObjectId(result));
    }

    Tag Repository::lookupTag(Result& result, const QString& refName)
    {
        return lookupTag(result, reference(result, refName).resolveToObjectId(result));
    }

    bool Repository::shouldIgnore(Result& result, const QString& filePath) const
    {
        GW_CD_CHECKED(Repository, false, result);

        int ignore = 0;

        result = git_status_should_ignore( &ignore, d->mRepo, GW_StringFromQt(filePath) );
        if( !result )
        {
            return false;
        }

        return ignore;
    }

    /**
     * @brief           Get a list of all remotes
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return          A list of Git::Remote objects containing all remotes of this repository.
     *
     */
    Remote::List Repository::allRemotes(Result& result) const
    {
        GW_CD_CHECKED(Repository, Remote::List(), result);

        git_strarray arr;
        result = git_remote_list( &arr, d->mRepo );
        if (!result) {
            return Remote::List();
        }

        Remote::List remotes;
        for (size_t i = 0; i < arr.count; i++) {
            git_remote* remote = nullptr;
            result = git_remote_lookup(&remote, d->mRepo, arr.strings[i]);
            if (!result) {
                git_strarray_free(&arr);
                return Remote::List();
            }
            Remote rm = new Remote::Private(const_cast<Private*>(d), remote);
            remotes.append(rm);
        }

        git_strarray_free(&arr);
        return remotes;
    }

    /**
     * @brief           Get a list of all remotes names
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return          A QStringList containing all names of the remotes of this repository.
     *
     */
    QStringList Repository::allRemoteNames( Result& result ) const
    {
        GW_CD_CHECKED(Repository, QStringList(), result);

        Internal::StrArray arr;
        result = git_remote_list( arr, d->mRepo );
        GW_CHECK_RESULT( result, QStringList() );

        return arr.strings();
    }

    /**
     * @brief           Find a remote by its name
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       remoteName  The name of the remote to find.
     *
     * @return          The Git::Remote that was found or an invalid Git::Remote object in case of
     *                  any error.
     *
     */
    Remote Repository::remote(Result& result, const QString& remoteName) const
    {
        GW_CD_CHECKED(Repository, Remote(), result);

        git_remote* remote = nullptr;
        result = git_remote_lookup( &remote, d->mRepo, GW_StringFromQt(remoteName) );

        if( !result )
        {
            return Remote();
        }

        return new Remote::Private(const_cast<Private*>(d), remote);
    }

    namespace Internal
    {

        struct cb_enum_submodules_t
        {
            RepositoryPrivate*  repo;
            Submodule::List     subs;
        };

        static int cb_enum_submodules(git_submodule* sm, const char* name, void* payload)
        {
            cb_enum_submodules_t* d = static_cast<cb_enum_submodules_t*>(payload);
            Q_ASSERT(d && name);

            d->subs.append(new SubmodulePrivate(d->repo, GW_StringToQt(name)));
            return 0;
        }

        static int cb_enum_submodule_names(git_submodule* sm, const char* name, void* payload)
        {
            QStringList* sl = static_cast<QStringList*>(payload);
            Q_ASSERT(sl && name);
            sl->append(GW_StringToQt(name));
            return 0;
        }

    }

    Submodule::List Repository::submodules( Result& result )
    {
        GW_D_CHECKED(Repository, Submodule::List(), result);
        Internal::cb_enum_submodules_t data = { d };

        result = git_submodule_foreach( d->mRepo, &Internal::cb_enum_submodules, &data );
        GW_CHECK_RESULT( result, Submodule::List() );

        return data.subs;
    }

    QStringList Repository::submoduleNames(Result& result) const
    {
        GW_CD_CHECKED(Repository, QStringList(), result);

        QStringList names;

        result = git_submodule_foreach(d->mRepo, &Internal::cb_enum_submodule_names, &names);
        GW_CHECK_RESULT( result, QStringList() );

        return names;
    }

    Submodule Repository::submodule(Result& result, const QString& name) const
    {
        GW_CD_CHECKED(Repository, Submodule(), result);

        if (submoduleNames(result).contains(name)) {
            return new Submodule::Private(const_cast<Private*>(d), name);
        }

        return Submodule();
    }

    CommitOperation* Repository::commitOperation(Result& result, const QString& msg)
    {
        return headBranch(result).commitOperation( result, index(result), msg );
    }

    /**
     * @brief           Detach the current HEAD
     *
     * Detachs the HEAD. If HEAD is already detached, this is a no-op and successful. If HEAD points
     * to a commit, it will be detached at that commit. If it points to a tag, the tag will be
     * peeled into a commit and HEAD is detached at the commit. It is an error, if HEAD points
     * somewhere else.
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @return          `true` on success, `false` otherwise.
     *
     */
    bool Repository::detachHead(Result& result)
    {
        GW_D_CHECKED(Repository, false, result);

        result = git_repository_detach_head(d->mRepo); //MEH
        return result;
    }

    /**
     * @brief           Set the HEAD to follow a branch
     *
     * @param[in,out]   result      A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       branchName  The (possibly full qualified) reference name of the branch to
     *                              follow. The branch doesn't need to exist. If it doesn't exist,
     *                              the HEAD will become orphaned but point to the branch once it is
     *                              created.
     *
     */
    void Repository::setHEAD(Result& result, const QString& branchName)
    {
        GW_D_CHECKED(Repository, void(), result);

        result = git_repository_set_head(d->mRepo, GW_StringFromQt(branchName));
    }

    /**
     * @brief           Set the HEAD to follow a branch
     *
     * @param[in,out]   result      A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       branch      The branch to follow.
     *
     */
    void Repository::setHEAD(Result& result, const BranchRef& branch)
    {
        GW_CHECK_RESULT( result, void() );

        if ( !branch.isValid() ) {
            result.setInvalidObject();
            return;
        }

        setHEAD(result, branch.name());
    }


    /**
     * @brief           Set the HEAD detached to a commit
     *
     * @param[in,out]   result  A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       sha     The has of the commit to point to.
     *
     */
    void Repository::setDetachedHEAD(Result& result, const ObjectId& sha)
    {
        GW_D_CHECKED(Repository, void(), result);
        result = git_repository_set_head_detached(d->mRepo, Private::sha(sha));
    }

    /**
     * @brief       Access the superproject for this repository
     *
     * @return      If this repository was opened via Submodule::subRepository(), the Repository
     *              that owns the Submodule object from which this repository was opened. Otherwise
     *              an invalid Repository() object will be returned.
     *
     */
    Repository Repository::superproject() const
    {
        return superprojectSubmodule().repository();
    }

    /**
     * @brief       Access the superproject's submodule for this repository
     *
     * @return      If this repository was opened via Submodule::subRepository(), this will return
     *              the Submodule object from which it was opened.
     *
     */
    Submodule Repository::superprojectSubmodule() const
    {
        GW_CD(Repository);
        if (!d) {
            return Submodule();
        }

        if (!d->openedFrom.isValid()) {
            return Submodule();
        }

        return d->openedFrom;
    }

    Reference Repository::reference(Result& result, const QString& refName, bool dwim)
    {
        GW_D_CHECKED(Repository, Reference(), result);

        QString name = refName;

        git_reference* ref = nullptr;
        if (dwim) {
            result = git_reference_dwim(&ref, d->mRepo, GW_StringFromQt(refName));

            if (!result) {
                return Reference();
            }

            name = GW_StringToQt(git_reference_name(ref));
        }
        else {
            result = git_reference_lookup( &ref, d->mRepo, GW_StringFromQt(refName) );

            if (!result) {
                return Reference();
            }
        }

        return Reference::Private::createRefObject(result, d, name, ref);
    }

    BranchRef Repository::branchRef(Result& result, const QString& branchName)
    {
        return reference(result, branchName, true).asBranch();
    }

    TagRef Repository::tagRef(Result& result, const QString& tagName)
    {
        return reference(result, tagName, true).asTag();
    }

    NoteRef Repository::noteRef(Result& result, const QString& noteName)
    {
        // dwim doesn't work for notes
        return reference(result, QLatin1Literal("refs/notes/") % noteName).asNote();
    }

    /**
     * @brief       Calculates the relation between two commits
     *
     * This checks how two commits are related to each other. It is usually used in the context of
     * a local branch following a remote branch (known as an @em upstream branch), but is not
     * limited to that use case.
     *
     * In that context, @a idLocal is the id of the commit that your local branch is pointing to and
     * @a idRemote the id that the remote branch is pointing to.
     *
     * The result can then be:
     * - ahead and behind are zero: There are no new commits to download (fetch) and you do not have
     *   local work that has to be pushed.
     *
     * - ahead is zero, behind is greater than zero: Someone else pushed to the branch and you have
     *   not pulled yet.
     *
     * - behind is zero, ahead is greater than zero: You have local commits that can safely be
     *   pushed to the remote.
     *
     * - ahead and behind are greater than zero: Different situations can lead to this. The most
     *   obvious one is that you are preparing to push your changes, but someone else has pushed
     *   already. The branches involved are said to "have diverged". This is also likely to be the
     *   case after a rebase.
     *
     * @param[in,out]   result      A Result object; see @ref GitWrapErrorHandling
     *
     * @param[in]       idLocal     The id of the local commit
     *
     * @param[in]       idRemote    The id of the remote commit
     *
     * @param[out]      ahead       The number of commits that @a idLocal is ahead of @a idRemote
     *                              will be placed into this.
     *
     * @param[out]      behind      The number of commits that @a idLocal is behind of @a idRemote
     *                              will be placed into this.
     */
    void Repository::calculateDivergence(Result& result,
                                         const ObjectId& idLocal, const ObjectId& idRemote,
                                         size_t& ahead, size_t& behind) const
    {
        GW_CD_CHECKED_VOID(Repository, result);

        result = git_graph_ahead_behind(&ahead, &behind, d->mRepo,
                                        Internal::ObjectId2git(idLocal),
                                        Internal::ObjectId2git(idRemote));
    }

}
