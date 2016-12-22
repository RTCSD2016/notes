VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   2475
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6255
   LinkTopic       =   "Form1"
   ScaleHeight     =   2475
   ScaleWidth      =   6255
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton StopButton 
      Caption         =   "Stop"
      Height          =   315
      Left            =   720
      TabIndex        =   12
      Top             =   1680
      Width           =   735
   End
   Begin VB.TextBox DoubleFromClient 
      Height          =   285
      Left            =   1440
      LinkItem        =   "DoubleFromClient"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   10
      Text            =   "1.3e7"
      Top             =   840
      Width           =   1695
   End
   Begin VB.TextBox Counter 
      Height          =   285
      Left            =   4800
      LinkItem        =   "Counter"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   8
      Text            =   "0"
      Top             =   1800
      Width           =   1455
   End
   Begin VB.TextBox StringFromServer 
      Height          =   285
      Left            =   4800
      LinkItem        =   "StringFromServer"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   6
      Text            =   "string"
      Top             =   1320
      Width           =   1455
   End
   Begin VB.TextBox IntFromServer 
      Height          =   285
      Left            =   4800
      LinkItem        =   "IntFromServer"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   4
      Text            =   "0"
      Top             =   840
      Width           =   1455
   End
   Begin VB.TextBox FloatFromServer 
      Height          =   285
      Left            =   4800
      LinkItem        =   "FloatFromServer"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   2
      Text            =   "0"
      Top             =   360
      Width           =   1455
   End
   Begin VB.TextBox LongFromClient 
      Height          =   285
      Left            =   1440
      LinkItem        =   "LongFromClient"
      LinkTopic       =   "Testing|Server Data"
      TabIndex        =   1
      Text            =   "2300"
      Top             =   360
      Width           =   1695
   End
   Begin VB.Label Label6 
      Caption         =   "DoubleFromClient"
      Height          =   255
      Left            =   0
      TabIndex        =   11
      Top             =   840
      Width           =   1335
   End
   Begin VB.Label Label5 
      Caption         =   "Counter"
      Height          =   255
      Left            =   3480
      TabIndex        =   9
      Top             =   1800
      Width           =   1215
   End
   Begin VB.Label Label4 
      Caption         =   "StringFromServer"
      Height          =   255
      Left            =   3480
      TabIndex        =   7
      Top             =   1320
      Width           =   1215
   End
   Begin VB.Label Label3 
      Caption         =   "IntFromServer"
      Height          =   255
      Left            =   3480
      TabIndex        =   5
      Top             =   840
      Width           =   1215
   End
   Begin VB.Label Label2 
      Caption         =   "FloatFromServer"
      Height          =   255
      Left            =   3480
      TabIndex        =   3
      Top             =   360
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "LongFromClient"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   360
      Width           =   1215
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub DoubleFromClient_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        DoubleFromClient.LinkPoke
    End If
End Sub

Private Sub DoubleFromClient_LostFocus()
    DoubleFromClient.LinkPoke
End Sub

Private Sub LongFromClient_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        LongFromClient.LinkPoke
    End If
End Sub

Private Sub LongFromClient_LostFocus()
    LongFromClient.LinkPoke
End Sub

Private Sub Form_Load()
    On Error GoTo NoSource
    
    LongFromClient.LinkMode = 2
    DoubleFromClient.LinkMode = 2
    FloatFromServer.LinkMode = 1
    IntFromServer.LinkMode = 1
    StringFromServer.LinkMode = 1
    Counter.LinkMode = 1
    Exit Sub
    
NoSource:
    If Err.Number = 282 Then
        MsgBox ("DDE Source not active")
        Resume
    Else
        MsgBox ("Unknown error -- will Exit")
        End
    End If
    
End Sub
Private Sub StopButton_Click()
    End
End Sub
