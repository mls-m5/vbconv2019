VERSION 5.00
Begin VB.Form frmEdit 
   AutoRedraw      =   -1  'True
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Form1"
   ClientHeight    =   5490
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6225
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   366
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   415
   StartUpPosition =   3  'Windows Default
   Begin VB.Line Line3 
      BorderStyle     =   3  'Dot
      Visible         =   0   'False
      X1              =   96
      X2              =   152
      Y1              =   96
      Y2              =   144
   End
   Begin VB.Line OffY 
      BorderStyle     =   4  'Dash-Dot
      X1              =   0
      X2              =   440
      Y1              =   256
      Y2              =   256
   End
   Begin VB.Line OffX 
      BorderStyle     =   4  'Dash-Dot
      X1              =   280
      X2              =   280
      Y1              =   0
      Y2              =   376
   End
   Begin VB.Line Line2 
      X1              =   0
      X2              =   408
      Y1              =   176
      Y2              =   176
   End
   Begin VB.Line Line1 
      X1              =   200
      X2              =   200
      Y1              =   368
      Y2              =   0
   End
End
Attribute VB_Name = "frmEdit"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Type LineType
    X1 As Long
    Y1 As Long
    X2 As Long
    Y2 As Long
End Type
Private Type LinePicture
    LineCount As Long
    Lines() As LineType
End Type

Private Type ShipType
    Acceration As Single
    Friction As Single
    Picture As Long
    Armor As Long
    Radie As Long
    StandardWeapon As Long
    Cost As Currency
End Type


Private Type WeaponType
    FireRate As Long
    ReloadTime As Long
    Ammo As Long
    Cluster As Long
    Speed As Long
    Acurrency As Single
    Picture As Long
    Size As Long
    Damage As Long
    Cost As Currency
End Type

Dim Lpicture As LinePicture, Ppicture As LinePicture, CX As Long, CY As Long, Zoom As Long

Sub DrawPicture()
    Dim i As Long
    Cls
    For i = 0 To Lpicture.LineCount
        With Lpicture.Lines(i)
            Line (.X1 * Zoom + 200, .Y1 * Zoom + 176)-(.X2 * Zoom + 200, .Y2 * Zoom + 176)
        End With
    Next
    CurrentX = 10
    CurrentY = 10
    Print "zooming: " & Zoom
End Sub

Sub AddLine(X1 As Long, Y1 As Long, X2 As Long, Y2 As Long)
    Dim CLines() As LineType, i As Long
    CLines = Lpicture.Lines
    If Lpicture.LineCount > -1 Then
        ReDim Lpicture.Lines(Lpicture.LineCount + 1)
        For i = 0 To Lpicture.LineCount
            Lpicture.Lines(i) = CLines(i)
        Next
    Else
        Lpicture.LineCount = -1
        ReDim Lpicture.Lines(0)
    End If
    Lpicture.LineCount = Lpicture.LineCount + 1
    
    With Lpicture.Lines(Lpicture.LineCount)
        .X1 = X1 / Zoom
        .Y1 = Y1 / Zoom
        .X2 = X2 / Zoom
        .Y2 = Y2 / Zoom
    End With
    DrawPicture
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyAdd Then
        Zoom = Zoom + 1
        DrawPicture
    ElseIf KeyCode = vbKeySubtract Then
        Zoom = Zoom - 1
        If Zoom < 1 Then Zoom = 1
        DrawPicture
    ElseIf Shift = 2 Then
        If KeyCode = vbKeyZ Then
            Dim CPicture  As LinePicture
            CPicture = Lpicture
            Lpicture = Ppicture
            Ppicture = CPicture
            DrawPicture
        ElseIf KeyCode = vbKeyO Then
            Zoom = 5
            Lpicture.LineCount = -1
            Dim FileName As String
            FileName = InputBox("Vart vill du ladda?")
            If Not FileName = "" Then
                Open "grafik\" & FileName For Binary As #1
                Get #1, , Lpicture
                Close #1
            End If
            DrawPicture
        ElseIf KeyCode = vbKeyN Then
            NewShipType
        ElseIf KeyCode = vbKeyW Then
            NewWeaponType
        End If
    End If
End Sub

Private Sub Form_Load()
    Me.Show
    Zoom = 5
    Lpicture.LineCount = -1
    DrawPicture
End Sub

Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    CX = X
    CY = Y
    With Line3
        .X1 = X
        .Y1 = Y
        .Visible = 1
    End With
End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    With OffX
        .X1 = X
        .X2 = X
    End With
    With OffY
        .Y1 = Y
        .Y2 = Y
    End With
    If Button Then
        With Line3
            .X2 = X
            .Y2 = Y
        End With
    End If
End Sub

Private Sub Form_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Ppicture = Lpicture
    AddLine CX - 200, CY - 176, X - 200, Y - 176
    Line3.Visible = 0
    If Shift = 2 Then
        AddLine -(CX - 200), CY - 176, -(X - 200), Y - 176
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim FileName As String
    FileName = InputBox("Vart vill du spara?")
    If Not FileName = "" Then
        'If Dir("grafik\" & FileName) <> "" Then Kill FileName
        Open "grafik\" & FileName For Binary As #1
        Put #1, , Lpicture
        Close #1
    End If
End Sub

Sub NewShipType()
    Dim Ship As ShipType, FileName As String
    FileName = InputBox("Var?")
    If FileName = "" Then
        Exit Sub
    Else
        Open "skepp\" & FileName & ".ship" For Binary As #1
        Get #1, , Ship
        Close #1
    End If
    With Ship
        .Acceration = InputBox("Acceration:", , .Acceration)
        .Friction = InputBox("Friktion:", , .Friction)
        .Armor = InputBox("Sköld:", , .Armor)
        .Radie = InputBox("Radie:", , .Radie)
        .StandardWeapon = InputBox("Standardvapen:", , .StandardWeapon)
        .Cost = InputBox("Kostnad:", , .Cost)
    End With
    If FileName > "" Then
        Open "skepp\" & FileName & ".ship" For Binary As #1
        Put #1, , Ship
        Close #1
    End If
End Sub

Sub NewWeaponType()
    Dim Weapon As WeaponType, FileName As String
    FileName = InputBox("Var?")
    If FileName = "" Then
        Exit Sub
    Else
        Open "vapen\" & FileName & ".weapon" For Binary As #1
        Get #1, , Weapon
        Close #1
    End If
    With Weapon
        .Acurrency = InputBox("Träffsäkerhet:", , .Acurrency)
        .Ammo = InputBox("Ammunition:", , .Ammo)
        .Cluster = InputBox("Splitter:", , .Cluster + 1) - 1
        If .Cluster < 0 Then .Cluster = 0
        .Damage = InputBox("Skada:", , .Damage)
        .FireRate = InputBox("Eldhastighet:", , .FireRate)
        .Picture = InputBox("Bild:", , .Picture)
        .ReloadTime = InputBox("Omladdningstid:", , .ReloadTime)
        .Size = InputBox("Storlek:", , .Size)
        .Speed = InputBox("Fart:", , .Speed)
        .Cost = InputBox("Kostnad:", , .Cost)
    End With
    If FileName > "" Then
        Open "vapen\" & FileName & ".weapon" For Binary As #1
        Put #1, , Weapon
        Close #1
    End If
End Sub
