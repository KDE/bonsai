if (LIBGIT2_FOUND)
    include_directories(${LIBGIT2_INCLUDE_DIR})
    add_definitions(-DWITH_LIBGIT2)
endif()


SET( SRC_FILES

    Base.cpp
    Blob.cpp
    BranchRef.cpp
    ChangeListConsumer.cpp
    Commit.cpp
    Config.cpp
    Diff.cpp
    DiffList.cpp
    GitWrap.cpp
    Index.cpp
    IndexConflict.cpp
    IndexConflicts.cpp
    IndexEntry.cpp
    NoteRef.cpp
    Object.cpp
    ObjectId.cpp
    PatchConsumer.cpp
    RefLog.cpp
    RefName.cpp
    RefSpec.cpp
    Reference.cpp
    Remote.cpp
    RepoObject.cpp
    Repository.cpp
    Result.cpp
    RevisionWalker.cpp
    Signature.cpp
    Submodule.cpp
    Tag.cpp
    TagRef.cpp
    Tree.cpp
    TreeBuilder.cpp
    TreeEntry.cpp

    Events/IGitEvents.cpp
    Events/Private/GitEventCallbacks.cpp

    Operations/BaseOperation.cpp
    Operations/CheckoutOperation.cpp
    Operations/CloneOperation.cpp
    Operations/CommitOperation.cpp
    Operations/RemoteOperations.cpp

    Operations/Private/WorkerThread.cpp
    )

SET( PUB_HDR_FILES

    Base.hpp
    Blob.hpp
    BranchRef.hpp
    ChangeListConsumer.hpp
    Commit.hpp
    Config.hpp
    Diff.hpp
    DiffList.hpp
    FileInfo.hpp
    GitWrap.hpp
    Index.hpp
    IndexConflict.hpp
    IndexConflicts.hpp
    IndexEntry.hpp
    NoteRef.hpp
    Object.hpp
    ObjectId.hpp
    PatchConsumer.hpp
    RefLog.hpp
    RefName.hpp
    RefSpec.hpp
    Reference.hpp
    Remote.hpp
    RepoObject.hpp
    Repository.hpp
    Result.hpp
    RevisionWalker.hpp
    Signature.hpp
    Submodule.hpp
    Tag.hpp
    TagRef.hpp
    Tree.hpp
    TreeBuilder.hpp
    TreeEntry.hpp

    Operations/BaseOperation.hpp
    Operations/CheckoutOperation.hpp
    Operations/CloneOperation.hpp
    Operations/CommitOperation.hpp
    Operations/Providers.hpp
    Operations/RemoteOperations.hpp

    Events/IGitEvents.hpp
    )

SET( PRI_HDR_FILES

    Private/BasePrivate.hpp
    Private/BlobPrivate.hpp
    Private/BranchRefPrivate.hpp
    Private/CommitPrivate.hpp
    Private/ConfigPrivate.hpp
    Private/DiffPrivate.hpp
    Private/GitWrapPrivate.hpp
    Private/IndexConflictPrivate.hpp
    Private/IndexEntryPrivate.hpp
    Private/IndexPrivate.hpp
    Private/ObjectPrivate.hpp
    Private/NoteRefPrivate.hpp
    Private/ReferencePrivate.hpp
    Private/RefLogPrivate.hpp
    Private/RefLogEntryPrivate.hpp
    Private/RefNamePrivate.hpp
    Private/RemotePrivate.hpp
    Private/RepoObjectPrivate.hpp
    Private/RepositoryPrivate.hpp
    Private/RevisionWalkerPrivate.hpp
    Private/SubmodulePrivate.hpp
    Private/TagPrivate.hpp
    Private/TagRefPrivate.hpp
    Private/TreeBuilderPrivate.hpp
    Private/TreeEntryPrivate.hpp
    Private/TreePrivate.hpp

    Events/Private/GitEventCallbacks.hpp

    Operations/Private/BaseOperationPrivate.hpp
    Operations/Private/CheckoutOperationPrivate.hpp
    Operations/Private/CloneOperationPrivate.hpp
    Operations/Private/RemoteOperationsPrivate.hpp
    Operations/Private/WorkerThread.hpp
    )

SET( HDR_FILES ${PRI_HDR_FILES} ${PUB_HDR_FILES} )
#set( GitWrapHeaders ${PUB_HDR_FILES} )

#QT_MOC( MOC_FILES ${HDR_FILES} )

add_library(
    GitWrap SHARED

    ${SRC_FILES}
    ${HDR_FILES}
    ${MOC_FILES}
    )

target_include_directories(GitWrap PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(
    GitWrap
    PRIVATE
    Qt5::Core
    ${LIBGIT2_LIBRARIES}
    )


install(TARGETS GitWrap ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
