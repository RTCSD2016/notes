VERSION 5.00
Begin VB.Form SetupForm 
   Caption         =   "Setup Screen"
   ClientHeight    =   2625
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   2625
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox cStartFlag 
      Height          =   285
      Left            =   240
      LinkItem        =   "dde_start_op"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   14
      Text            =   "0"
      Top             =   2040
      Visible         =   0   'False
      Width           =   495
   End
   Begin VB.TextBox cScreenNo 
      Height          =   285
      Left            =   240
      LinkItem        =   "dde_ScreenNo"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   13
      Text            =   "0"
      Top             =   1680
      Visible         =   0   'False
      Width           =   495
   End
   Begin VB.TextBox cTR 
      Height          =   285
      Left            =   3360
      LinkItem        =   "dde_tickrate"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   12
      Text            =   "0.0001"
      Top             =   1200
      Width           =   1095
   End
   Begin VB.TextBox cMAX 
      Height          =   285
      Left            =   3360
      LinkItem        =   "dde_max"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   11
      Text            =   "1"
      Top             =   720
      Width           =   1095
   End
   Begin VB.TextBox cMIN 
      Height          =   285
      Left            =   3360
      LinkItem        =   "dde_min"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   10
      Text            =   "0"
      Top             =   240
      Width           =   1095
   End
   Begin VB.CommandButton cStartControl 
      Caption         =   "Start Control"
      Height          =   375
      Left            =   1800
      TabIndex        =   6
      Top             =   1920
      Width           =   1215
   End
   Begin VB.TextBox cKD 
      Height          =   285
      Left            =   600
      LinkItem        =   "dde_kd"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   5
      Text            =   "0"
      Top             =   1200
      Width           =   975
   End
   Begin VB.TextBox cKI 
      Height          =   285
      Left            =   600
      LinkItem        =   "dde_ki"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   4
      Text            =   "5"
      Top             =   720
      Width           =   975
   End
   Begin VB.TextBox cKP 
      Height          =   285
      Left            =   600
      LinkItem        =   "dde_kp"
      LinkTopic       =   "DDEHeater|HeaterData"
      TabIndex        =   3
      Text            =   "10.5"
      Top             =   240
      Width           =   975
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
      Caption         =   "Min"
      Height          =   255
      Left            =   2760
      TabIndex        =   9
      Top             =   240
      Width           =   495
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Caption         =   "Max"
      Height          =   255
      Left            =   2880
      TabIndex        =   8
      Top             =   720
      Width           =   375
   End
   Begin VB.Label Label4 
      Alignment       =   1  'Right Justify
      Caption         =   "Tickrate"
      Height          =   255
      Left            =   2520
      TabIndex        =   7
      Top             =   1200
      Width           =   735
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Kd"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   1200
      Width           =   375
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Ki"
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   720
      Width           =   375
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Kp"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   240
      Width           =   375
   End
End
Attribute VB_Name = "SetupForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private scrn, prevscrn, mb

Private Sub ScreenSelect()
    'Hide previous screen
    If prevscrn = 0 Then
        SetupForm.Visible = False
    ElseIf prevscrn = 1 Then
        OperationForm.Visible = False
    End If
    
    'Show current screen
    scrn = Val(cScreenNo.Text)
    If scrn = 0 Then
        SetupForm.Visible = True
        'Send out initial values
        cKP.LinkPoke
        cKI.LinkPoke
        cKD.LinkPoke
        cMIN.LinkPoke
        cMAX.LinkPoke
        cTR.LinkPoke
        cStartFlag.LinkPoke
    ElseIf scrn = 1 Then
        OperationForm.Visible = True
        'OperationsPoke 'Send values to C++ program
    End If
    
    
    prevscrn = scrn
End Sub

Private Sub cKD_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cKD.LinkPoke
    End If
End Sub

Private Sub cKD_LostFocus()
        cKD.LinkPoke
End Sub

Private Sub cKI_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cKI.LinkPoke
    End If
End Sub

Private Sub cKI_LostFocus()
        cKI.LinkPoke
End Sub

Private Sub cKP_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cKP.LinkPoke
    End If
End Sub

Private Sub cKP_LostFocus()
        cKP.LinkPoke
End Sub

Private Sub cMAX_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cMAX.LinkPoke
    End If
End Sub

Private Sub cMAX_LostFocus()
        cMAX.LinkPoke
End Sub

Private Sub cMIN_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cMIN.LinkPoke
    End If
End Sub

Private Sub cMIN_LostFocus()
        cMIN.LinkPoke
End Sub

Private Sub cScreenNo_Change()
    ScreenSelect
End Sub

Private Sub cStartControl_Click()
    cStartFlag.Text = "1"
    cStartFlag.LinkPoke
End Sub

Private Sub cTR_KeyPress(KeyAscii As Integer)
    If KeyAscii = vbKeyReturn Then
        KeyAscii = 0
        cTR.LinkPoke
    End If
End Sub

Private Sub cTR_LostFocus()
        cTR.LinkPoke
End Sub

Private Sub Form_Load()
    On Error GoTo NoSource
    
    cKP.LinkMode = 2 'Set DDE mode to manual for
        'values being sent to the C++ program
    cKI.LinkMode = 2
    cKD.LinkMode = 2
    cMIN.LinkMode = 2
    cMAX.LinkMode = 2
    cTR.LinkMode = 2
    cStartFlag.LinkMode = 2
    cScreenNo.LinkMode = 1  'DDE mode automatic to
        ' receive information here
    prevscrn = -1 'Previous screen number
    
    Load OperationForm
    ScreenSelect
    
    Exit Sub

NoSource:
    If Err.Number = 282 Then
        mb = MsgBox("No DDE Source <Setup>", vbRetryCancel)
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
            MsgBox ("Program will exit. <Setup>")
            End
        End If
    End If
End Sub
