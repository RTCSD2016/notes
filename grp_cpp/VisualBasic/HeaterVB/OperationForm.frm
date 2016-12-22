VERSION 5.00
Begin VB.Form OperationForm 
   Caption         =   "Operating Screen"
   ClientHeight    =   3030
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   3930
   LinkTopic       =   "Form1"
   ScaleHeight     =   3030
   ScaleWidth      =   3930
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox cStopFlag 
      Height          =   285
      Left            =   3120
      LinkItem        =   "dde_stop_prog"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   11
      Text            =   "0"
      Top             =   2400
      Visible         =   0   'False
      Width           =   255
   End
   Begin VB.TextBox cCtrl 
      Height          =   285
      Left            =   1800
      LinkItem        =   "dde_mcntrl"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   10
      Text            =   "0"
      Top             =   1920
      Width           =   1455
   End
   Begin VB.TextBox cSet 
      Height          =   285
      Left            =   1800
      LinkItem        =   "dde_setpoint"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   9
      Text            =   "0"
      Top             =   1560
      Width           =   1455
   End
   Begin VB.TextBox cTemp 
      Height          =   285
      Left            =   1800
      LinkItem        =   "dde_temperature"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   8
      Text            =   "0"
      Top             =   1200
      Width           =   1455
   End
   Begin VB.TextBox cTime 
      Height          =   285
      Left            =   1800
      LinkItem        =   "dde_Time"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   7
      Text            =   "0"
      Top             =   840
      Width           =   1455
   End
   Begin VB.TextBox cFinal 
      Height          =   285
      Left            =   1800
      LinkItem        =   "dde_temp_final"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   6
      Text            =   "0.35"
      Top             =   240
      Width           =   1455
   End
   Begin VB.CommandButton cStop 
      Caption         =   "Stop"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1680
      TabIndex        =   5
      Top             =   2400
      Width           =   855
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Caption         =   "Controller Output"
      Height          =   255
      Left            =   360
      TabIndex        =   4
      Top             =   1920
      Width           =   1335
   End
   Begin VB.Label Label4 
      Alignment       =   1  'Right Justify
      Caption         =   "Setpoint"
      Height          =   255
      Left            =   960
      TabIndex        =   3
      Top             =   1560
      Width           =   735
   End
   Begin VB.Line Line1 
      X1              =   240
      X2              =   3600
      Y1              =   600
      Y2              =   600
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Temperature"
      Height          =   255
      Left            =   720
      TabIndex        =   2
      Top             =   1200
      Width           =   975
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Time"
      Height          =   255
      Left            =   840
      TabIndex        =   1
      Top             =   840
      Width           =   855
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Final Temp. Setpoint"
      Height          =   255
      Left            =   240
      TabIndex        =   0
      Top             =   240
      Width           =   1455
   End
End
Attribute VB_Name = "OperationForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private mb

Public Sub OperationsPoke()
    cFinal.LinkPoke
    cStopFlag.LinkPoke
End Sub

Private Sub cFinal_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cFinal.LinkPoke
    End If
End Sub

Private Sub cFinal_LostFocus()
    cFinal.LinkPoke 'Send out value
End Sub

Private Sub cStop_Click()
    cStopFlag.Text = "1" 'Set values of stop flag
    cStopFlag.LinkPoke  'Send it to the C++ program
    MsgBox ("Program will exit!")
    End  'Quit program
End Sub

Private Sub Form_Load()
    On Error GoTo NoSource
    cFinal.LinkMode = 2 'Set DDE mode to manual for
        'values being sent to the C++ program
    cStopFlag.LinkMode = 2
    cTime.LinkMode = 1  'DDE mode automatic to
        ' receive information here
    cTemp.LinkMode = 1
    cSet.LinkMode = 1
    cCtrl.LinkMode = 1
    cStopFlag.LinkMode = 1
    OperationsPoke  'Send initial values
    Exit Sub
    
NoSource:
    If Err.Number = 282 Then
        mb = MsgBox("No DDE Source <Op Form>", vbRetryCancel)
        If mb = vbRetry Then
            Resume
        Else
            MsgBox ("Program will exit.")
            End
        End If
    Else
        mb = MsgBox(Err.Description, vbRetryCancel)
        If mb = vbRetry Then
            Resume
        Else
            MsgBox ("Program will exit <Op>.")
            End
        End If
    End If
End Sub
