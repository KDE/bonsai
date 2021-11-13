import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

import org.maui.bonsai 1.0 as Bonsai

Maui.Page
{
    id: control

    property alias project: _project
    property alias url : _project.url

     Bonsai.Project
     {
         id: _project
     }

    title: project.title


    SwipeView
    {
        id: _swipeView
        anchors.fill: parent

        FB.FileBrowser
        {
            currentPath: control.project.url
        }
    }

}
