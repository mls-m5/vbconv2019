VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Ship"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit
Public XPos#, YPos#, Angle!
Public XSpeed As Single, YSpeed As Single
Public Player As ShipPilot
Public Radie As Long
Public Health As Long
Public ShipNumber As ShipTypeEnum
Dim Physis As ShipType, StartupPos As Position
Dim MPicture As LinePicture

Dim ReloadTime As Long, FireRate As Long, Ammo As Long, Weapons As WeaponCollection, Reload As Boolean
Dim WeaponChangeDelay As Long, WeaponSelected As Long

'////////////////////////////////////////////
    Dim ComputerVariable!
'////////////////////////////////////////////

Public Sub TimeTab()
    Dim i As Long, HitAngle!, Move As Long, Middle As Long
    i = (isShip(XPos, YPos, Radie, Me))
    If i > -1 Then
        With GetShip(i)
            HitAngle = GetAngle(.XPos - XPos, .YPos - YPos)
            Move = Sin(HitAngle) * (Radie + .Radie) / 2
            Middle = XPos + (.XPos - XPos) / 2
            XPos = Middle - Move
            .XPos = Middle + Move
            Move = Cos(HitAngle) * (Radie + .Radie) / 2
            Middle = YPos + (.YPos - YPos) / 2
            YPos = Middle + Move
            .YPos = Middle - Move
        End With
    End If
    XPos = XPos + XSpeed
    YPos = YPos + YSpeed
    XSpeed = XSpeed / Physis.Friction
    YSpeed = YSpeed / Physis.Friction
    PLLinePicture MPicture, XPos, YPos, Angle
    If FireRate > 0 Then FireRate = FireRate - 1
    If ReloadTime > 0 Then ReloadTime = ReloadTime - 1
    If Reload And ReloadTime < 1 Then
        Reload = 0
        Ammo = Weapons.Weapons(WeaponSelected).Ammo
    End If
    If Player = spPlayer Then
        SetScreenPos XPos, YPos, Angle
        If GetAsyncKeyState(vbKeyLeft) Then Angle = Angle - 0.2
        If GetAsyncKeyState(vbKeyRight) Then Angle = Angle + 0.2
        If GetAsyncKeyState(vbKeyW) Then
            XSpeed = XSpeed + Sin(Angle) * Physis.Acceration
            YSpeed = YSpeed - Cos(Angle) * Physis.Acceration
        End If
        If GetAsyncKeyState(vbKeyS) Then
            XSpeed = XSpeed - Sin(Angle) * Physis.Acceration
            YSpeed = YSpeed + Cos(Angle) * Physis.Acceration
        End If
        If GetAsyncKeyState(vbKeyA) Then
            XSpeed = XSpeed + Sin(Angle - PI / 2) * Physis.Acceration
            YSpeed = YSpeed - Cos(Angle - PI / 2) * Physis.Acceration
        End If
        If GetAsyncKeyState(vbKeyD) Then
            XSpeed = XSpeed + Sin(Angle + PI / 2) * Physis.Acceration
            YSpeed = YSpeed - Cos(Angle + PI / 2) * Physis.Acceration
        End If
        If GetAsyncKeyState(vbKeySpace) Then
            If FireRate < 1 And Not Reload Then Fire
        End If
        If WeaponChangeDelay > 0 Then
            WeaponChangeDelay = WeaponChangeDelay - 1
        Else
            If GetAsyncKeyState(vbKeyShift) Then
                WeaponSelected = WeaponSelected + 1
                If WeaponSelected > Weapons.WeaponCount - 1 Then WeaponSelected = 0
                Ammo = Weapons.Weapons(WeaponSelected).Ammo
                
                WeaponChangeDelay = 10
            End If
        End If
        If GetAsyncKeyState(vbKeyControl) Then
            Reload = True
            ReloadTime = Weapons.Weapons(WeaponSelected).ReloadTime
        End If
        If Angle > PI * 2 Then Angle = Angle - PI * 2
        If Angle < -PI * 2 Then Angle = Angle + PI * 2
        If isShip(XPos, YPos, 800, Me) < 0 Then
            Dim LastEnemyAngle As Single, LastEnemy As Ship
            Set LastEnemy = GetShip(0)
            If LastEnemy Is Me Then Set LastEnemy = GetShip(1)
            If Not LastEnemy Is Nothing Then
                LastEnemyAngle = GetAngle(LastEnemy.XPos - XPos, LastEnemy.YPos - YPos)
                PLLinePicture LoadedLPics(lpKula), XPos + Sin(LastEnemyAngle) * 20, YPos - Cos(LastEnemyAngle) * 20
            End If
        End If
        
        DrawLinePicture LoadedLPics(lpStatusbar), 20, 20, Health / Physis.Armor * PI * 2, Health / Physis.Armor * 16777215
        DrawLinePicture LoadedLPics(lpStatusbar), 60, 20, Ammo / Weapons.Weapons(WeaponSelected).Ammo * PI * 2, 16777215 / (ReloadTime + 1)
    ElseIf Player = spComputerHostile Or Player = spComputerHostileBase Then
        If PlayerShip Is Nothing Then Exit Sub
        Dim PlayerAngle As Single
        With PlayerShip
            'If Sqr((.XPos - XPos) ^ 2 + (.YPos - YPos) ^ 2) < 1000 Then
                PlayerAngle = GetAngle(.XPos - XPos, .YPos - YPos)
                If Int(Rnd * 5) = 1 And FireRate < 1 And Not Reload Then Fire
            'End If
            If PlayerAngle > Angle Then Angle = Angle + 0.2
            If PlayerAngle < Angle Then Angle = Angle - 0.2
            If PlayerAngle > Angle Then Angle = Angle + 0.01
            If PlayerAngle < Angle Then Angle = Angle - 0.01
            If Sqr((.XPos - XPos) ^ 2 + (.YPos - YPos) ^ 2) > 80 + Radie Then
                XSpeed = XSpeed + Sin(Angle) * Physis.Acceration
                YSpeed = YSpeed - Cos(Angle) * Physis.Acceration
            Else
                ComputerVariable = ComputerVariable + 0.1 + Rnd / 10
                XSpeed = XSpeed + Sin(ComputerVariable) * Physis.Acceration
                YSpeed = YSpeed - Cos(ComputerVariable) * Physis.Acceration
            End If
        End With
    ElseIf Player = spComputerFriendly Or Player = spComputerFriendlyBase Or Player = spComputerAntiHostile Then
        If ShipNumber = stMarket Then
            If GetAsyncKeyState(vbKeyControl) And GetAsyncKeyState(vbKeyD) Then
                Trade Me
            End If
        Else
            With StartupPos
                If Sqr((.X - XPos) ^ 2 + (.Y - YPos) ^ 2) < 300 Then
                    Angle = Angle + Rnd * 0.1 - 0.05
                Else
                    Angle = Angle + (GetAngle(.X - XPos, .Y - YPos) - Angle) / 3
                End If
            End With
        End If
        XSpeed = XSpeed + Sin(Angle) * Physis.Acceration
        YSpeed = YSpeed - Cos(Angle) * Physis.Acceration
    End If
End Sub

Public Sub Init(ByVal X&, ByVal Y&, ShipType As ShipTypeEnum, Optional ByVal Player2 As ShipPilot, Optional ByVal XSpeed2!, Optional ByVal YSpeed2!, Optional ByVal Angle2!)
    XPos = X
    YPos = Y
    With StartupPos
        .X = X
        .Y = Y
    End With
    XSpeed = XSpeed2
    YSpeed = YSpeed2
    Angle = Angle2
    Player = Player2
    If Player = spPlayer Then Set PlayerShip = Me
    
    Physis = ShipTypes(ShipType)
    ShipNumber = ShipType
    
    Health = Physis.Armor
    MPicture = ShipPictures(ShipNumber) 'Physis.Picture)
    Radie = Physis.Radie
    AddWeapon Weapons, Physis.StandardWeapon
    If Player = spComputerHostileBase And ShipNumber = stMarket Then Player = spNone
    Ammo = Weapons.Weapons(0).Ammo
End Sub

Public Sub Damage(Damage As Long)
    Health = Health - Damage
    If Health < 0 Then
        RemoveShip Me
        MakeLineTrash MPicture, XPos, YPos, Angle
    ElseIf Health > Physis.Armor Then
        Health = Physis.Armor
    End If
End Sub

Public Sub Fire()
    If Weapons.WeaponCount < 1 Then Exit Sub
    Dim i As Long
    With Weapons.Weapons(WeaponSelected)
        Dim Shot2 As Shot
        For i = 0 To .Cluster
            Set Shot2 = New Shot
            Shot2.Init XPos + Sin(Angle) * Radie, YPos - Cos(Angle) * Radie, .Speed, .Picture, Angle + ((Rnd - 0.5) / 2) / 2 * .Acurrency, 20, .Damage, , , , Me
            AddShot Shot2
        Next i
        Ammo = Ammo - 1
        If Ammo < 1 Then
            ReloadTime = .ReloadTime
            Reload = 1
        End If
        FireRate = .FireRate
    End With
End Sub

Public Sub AddShipWeapon(WeaponNumber As Weapontypes)
    AddWeapon Weapons, WeaponNumber
End Sub
