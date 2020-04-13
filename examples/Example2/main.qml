import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Example 2.0
Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Example 2")



    //instance the stellar gateway, it builds the StellarGateway object
    StellarGateway{
        id: testHorizonGateway;
    }

    Wallet{
        id:localWallet;
        gateway : testHorizonGateway;//assign gateway to wallet object, it calls the WRITE method of the property
    }


    ColumnLayout{

        anchors.fill: parent;
        anchors.margins: 5;
        RowLayout{
            Label{
                text:qsTr("Our account");
            }
            TextField{
                text: localWallet.publicAddress;//it binds publicAddress to text, each time publicAddress signal is emitted, it will be updated
                enabled:false;
                Layout.fillWidth: true;
            }
            Button{
                id:createButton;
                text:qsTr("Create");
                visible: localWallet.publicAddress=="";
                onClicked: {
                    localWallet.createRandom();
                }
            }
            Button{
                id:fundButton;
                visible: localWallet.publicAddress!="" && !localWallet.funded;
                text:qsTr("Fund");
                onClicked: {
                    localWallet.fund();
                }
            }
        }
        RowLayout{
            Label{
                text:qsTr("Balance");
            }
            TextField{
                text:localWallet.balance;
                enabled:false;
            }
        }
        PaymentGUI{
            sourceWallet: localWallet;
        }
    }
}
