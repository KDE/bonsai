import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui

AbstractButton
{
    id: control

    default property alias actions : _menu.contentData

    contentItem: Maui.ToolActions
    {
        autoExclusive: false
        expanded: true
        display: control.display
        checkable: false

        Action
        {
            icon: control.icon
            text: control.text
            onTriggered: control.clicked()
        }

        Action
        {
            onTriggered: _menu.popup()
//            icon.height: 8
//            icon.width: 8
            icon.name: "view-more-horizontal-symbolic"
            checked: _menu.visible
        }
    }

   data: Menu
    {
        id: _menu
    }
}
