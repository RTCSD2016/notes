VERSION 5.00
Begin VB.Form Master 
   Caption         =   "Master Screen"
   ClientHeight    =   1725
   ClientLeft      =   960
   ClientTop       =   5070
   ClientWidth     =   2415
   LinkTopic       =   "Form1"
   ScaleHeight     =   1725
   ScaleWidth      =   2415
   Begin VB.CommandButton Quit 
      Caption         =   "Quit"
      Height          =   375
      Left            =   600
      TabIndex        =   2
      Top             =   1080
      Width           =   735
   End
   Begin VB.TextBox ScreenNumber 
      Height          =   285
      Left            =   600
      TabIndex        =   0
      Text            =   "0"
      Top             =   600
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "Screen Number"
      Height          =   255
      Left            =   600
      TabIndex        =   1
      Top             =   360
      Width           =   1335
   End
End
Attribute VB_Name = "Master"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private scrn, prevscrn

Private Sub SelectScreen()
    scrn = Val(ScreenNumber.Text)
    
    ' Hide previous screen
    Select Case prevscrn
        Case 1
            Screen1.Visible = False
        Case 2
            MoveObject.Visible = False
    End Select
    
    ' Display current screen
    Select Case scrn
        Case 1
            Screen1.Visible = True
        Case 2
            MoveObject.Visible = True
    End Select

    prevscrn = scrn  'Remember current screen
End Sub

Private Sub Form_Load()
    prevscrn = 0
    SelectScreen
End Sub

Private Sub Quit_Click()
    End
End Sub

Private Sub ScreenNumber_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        SelectScreen
    End If

End Sub

Private Sub ScreenNumber_LostFocus()
    'SelectScreen
End Sub
