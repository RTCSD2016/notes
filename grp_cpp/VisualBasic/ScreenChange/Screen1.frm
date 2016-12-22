VERSION 5.00
Begin VB.Form Screen1 
   Caption         =   "Screen1"
   ClientHeight    =   1380
   ClientLeft      =   5265
   ClientTop       =   2370
   ClientWidth     =   3795
   LinkTopic       =   "Form2"
   ScaleHeight     =   1380
   ScaleWidth      =   3795
   Visible         =   0   'False
   Begin VB.TextBox y2x 
      Height          =   285
      Left            =   1320
      TabIndex        =   3
      Text            =   "No Value"
      Top             =   360
      Width           =   975
   End
   Begin VB.TextBox x 
      Height          =   285
      Left            =   240
      TabIndex        =   2
      Text            =   "1.3"
      Top             =   360
      Width           =   855
   End
   Begin VB.Label Label2 
      Caption         =   "Y = 2 * X"
      Height          =   255
      Left            =   1320
      TabIndex        =   1
      Top             =   120
      Width           =   735
   End
   Begin VB.Label Label1 
      Caption         =   "X"
      Height          =   255
      Left            =   360
      TabIndex        =   0
      Top             =   120
      Width           =   375
   End
End
Attribute VB_Name = "Screen1"
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
