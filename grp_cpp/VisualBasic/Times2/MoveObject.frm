VERSION 5.00
Begin VB.Form MoveObject 
   Caption         =   "MoveObject"
   ClientHeight    =   3345
   ClientLeft      =   5265
   ClientTop       =   5070
   ClientWidth     =   5070
   LinkTopic       =   "Form1"
   ScaleHeight     =   3345
   ScaleWidth      =   5070
   Begin VB.ComboBox ShapeType 
      Height          =   315
      ItemData        =   "MoveObject.frx":0000
      Left            =   3480
      List            =   "MoveObject.frx":0016
      TabIndex        =   9
      Text            =   "Type of Shape"
      Top             =   2520
      Width           =   1455
   End
   Begin VB.TextBox ObjWidth 
      Height          =   285
      Left            =   4080
      TabIndex        =   6
      Text            =   "0.2"
      Top             =   1800
      Width           =   735
   End
   Begin VB.TextBox ObjHeight 
      Height          =   285
      Left            =   4080
      TabIndex        =   5
      Text            =   "0.2"
      Top             =   1440
      Width           =   735
   End
   Begin VB.TextBox Pos_y 
      Height          =   285
      Left            =   4080
      TabIndex        =   3
      Text            =   "0"
      Top             =   1080
      Width           =   735
   End
   Begin VB.TextBox Pos_x 
      Height          =   285
      Left            =   4080
      TabIndex        =   1
      Text            =   "0"
      Top             =   720
      Width           =   735
   End
   Begin VB.CommandButton ApplyButton 
      Caption         =   "Apply"
      Height          =   375
      Left            =   4080
      TabIndex        =   0
      Top             =   120
      Width           =   915
   End
   Begin VB.Label Label5 
      Caption         =   "Shape"
      Height          =   255
      Left            =   3480
      TabIndex        =   10
      Top             =   2280
      Width           =   975
   End
   Begin VB.Label Label4 
      Caption         =   "Width"
      Height          =   255
      Left            =   3480
      TabIndex        =   8
      Top             =   1800
      Width           =   495
   End
   Begin VB.Label Label3 
      Caption         =   "Height"
      Height          =   255
      Left            =   3480
      TabIndex        =   7
      Top             =   1440
      Width           =   495
   End
   Begin VB.Label Label2 
      Caption         =   "Y"
      Height          =   255
      Left            =   3720
      TabIndex        =   4
      Top             =   1080
      Width           =   255
   End
   Begin VB.Label Label1 
      Caption         =   "X"
      Height          =   255
      Left            =   3720
      TabIndex        =   2
      Top             =   720
      Width           =   255
   End
   Begin VB.Shape Shape1 
      FillColor       =   &H000000FF&
      FillStyle       =   0  'Solid
      Height          =   1095
      Left            =   1560
      Shape           =   3  'Circle
      Top             =   1800
      Width           =   1095
   End
End
Attribute VB_Name = "MoveObject"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private x, y, ht, wdth, ShapeIndex

Private Sub PlaceObject()
    x = Val(Pos_x.Text) * 2000
    y = 2000 - Val(Pos_y.Text) * 2000
    ht = Val(ObjHeight.Text) * 2000
    wdth = Val(ObjWidth.Text) * 2000
    ShapeIndex = ShapeType.ListIndex
    Shape1.Shape = ShapeIndex
    Shape1.Left = x
    Shape1.Top = y
    
    Select Case ShapeIndex
        Case 1, 3, 5 'Square, circle, rounded square
          ' These shapes have only one
          ' dimension - height and width are set equal
          ' to the "height" as entered on the form
          Shape1.Height = ht
          Shape1.Width = ht
          ObjWidth.Text = Str(ht / 2000)
        Case 0, 2, 4 'Rectangle, oval, rounded rectangle
          Shape1.Height = ht
          Shape1.Width = wdth
    End Select
End Sub

Private Sub ApplyButton_Click()
    PlaceObject
End Sub

Private Sub Form_Load()
    ShapeType.ListIndex = 3 ' Circle as default
    PlaceObject
End Sub

