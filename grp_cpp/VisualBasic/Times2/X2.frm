VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Y=2X"
   ClientHeight    =   2445
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   ScaleHeight     =   2445
   ScaleWidth      =   2775
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox y2x 
      Height          =   285
      Left            =   840
      TabIndex        =   2
      Top             =   1320
      Width           =   975
   End
   Begin VB.TextBox x 
      Height          =   285
      Left            =   840
      TabIndex        =   0
      Text            =   "2.4"
      Top             =   480
      Width           =   975
   End
   Begin VB.Label Label2 
      Caption         =   "Y = 2 * X"
      Height          =   255
      Left            =   960
      TabIndex        =   3
      Top             =   1080
      Width           =   735
   End
   Begin VB.Label Label1 
      Caption         =   "X"
      Height          =   255
      Left            =   1080
      TabIndex        =   1
      Top             =   240
      Width           =   375
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private xx, y

Private Sub Mult2()
    xx = Val(x.Text)
    y = 2 * x
    y2x.Text = Str(y)
End Sub

Private Sub Form_Load()
    Mult2
End Sub

Private Sub x_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        Mult2
    End If
End Sub

Private Sub x_LostFocus()
    Mult2
End Sub
