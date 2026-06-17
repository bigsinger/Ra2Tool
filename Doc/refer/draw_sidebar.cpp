//从第561行开始就是观战的分支
char __userpurge sub_6A9540@<al>(StripClass *a1@<ecx>, wchar_t a2@<di>, char a3)
{
  char result; // al
  int v5; // eax
  HouseClass *v6; // ebx
  HouseClass *v7; // ebp
  int CameoCount; // eax
  int v9; // edi
  int *v10; // edx
  int v11; // eax
  int v12; // eax
  int TopRowIndex; // edx
  int x; // ebp
  int v15; // esi
  char v16; // bl
  int v17; // ebp
  int Index; // eax
  int v19; // eax
  int v20; // edx
  AbstractType ItemType; // ecx
  char *v22; // eax
  TechnoTypeClass *v23; // ebx
  bool v24; // al
  int Progress; // eax
  int v26; // ecx
  int *v27; // edi
  int v28; // eax
  int v29; // ecx
  unsigned int v30; // edi
  int v31; // eax
  DSurface_vtb_7E85D4 *vt; // edx
  int v33; // edi
  AbstractType v34; // eax
  int v35; // eax
  const wchar_t *v36; // ebx
  TechnoClass *Object; // ecx
  const wchar_t *String_734E60; // ebp
  int v39; // ebp
  int v40; // ebx
  int v41; // edi
  HouseClass *v42; // esi
  HouseTypeClass *Type; // eax
  int SideIndex; // eax
  int v45; // eax
  SHPReference *v46; // eax
  int v47; // edx
  int v48; // edx
  HouseTypeClass *v49; // eax
  int ArrayIndex2; // eax
  SHPReference *v51; // esi
  ConvertClass *v52; // eax
  int v53; // edx
  int v54; // edx
  int Height; // ecx
  ColorScheme *v56; // esi
  int v57; // eax
  const wchar_t *v58; // eax
  size_t v59; // eax
  size_t v60; // eax
  size_t v61; // eax
  size_t v62; // eax
  Point2D v63; // [esp-16h] [ebp-4A0h]
  Point2D v64; // [esp-16h] [ebp-4A0h]
  Point2D v65; // [esp-16h] [ebp-4A0h]
  char Naval; // [esp-Ah] [ebp-494h]
  __int16 v68; // [esp+10h] [ebp-47Ah]
  bool v69; // [esp+10h] [ebp-47Ah]
  char v70; // [esp+11h] [ebp-479h]
  bool Has_Completed; // [esp+12h] [ebp-478h]
  char v72; // [esp+13h] [ebp-477h]
  char v73; // [esp+14h] [ebp-476h]
  char v74; // [esp+15h] [ebp-475h]
  wchar_t *UIName; // [esp+16h] [ebp-474h]
  FactoryClass *PrimaryFactory_500510; // [esp+16h] [ebp-474h]
  Point2D a5; // [esp+1Ah] [ebp-470h] BYREF
  StripClass *v78; // [esp+22h] [ebp-468h]
  int v79; // [esp+26h] [ebp-464h]
  TechnoTypeClass *v80; // [esp+2Ah] [ebp-460h]
  char *v81; // [esp+2Eh] [ebp-45Ch]
  TechnoTypeClass *v82; // [esp+32h] [ebp-458h]
  signed int v83; // [esp+36h] [ebp-454h]
  int v84; // [esp+3Ah] [ebp-450h]
  const wchar_t *BufferCount; // [esp+3Eh] [ebp-44Ch]
  HouseClass *v86; // [esp+42h] [ebp-448h]
  SHPStruct *SHP; // [esp+46h] [ebp-444h]
  int v88; // [esp+4Ah] [ebp-440h]
  int i; // [esp+4Eh] [ebp-43Ch]
  RectangleStruct a4; // [esp+52h] [ebp-438h] BYREF
  Point2D v91; // [esp+62h] [ebp-428h] BYREF
  Point2D v92; // [esp+6Ah] [ebp-420h] BYREF
  Point2D v93; // [esp+72h] [ebp-418h] BYREF
  RectangleStruct v94; // [esp+7Ah] [ebp-410h] BYREF
  RectangleStruct v95; // [esp+8Ah] [ebp-400h] BYREF
  RectangleStruct v96; // [esp+9Ah] [ebp-3F0h] BYREF
  RectangleStruct Bounds; // [esp+AAh] [ebp-3E0h] BYREF
  Point2D v98; // [esp+BAh] [ebp-3D0h] BYREF
  int v99[2]; // [esp+C2h] [ebp-3C8h] BYREF
  Point2D v100; // [esp+CAh] [ebp-3C0h] BYREF
  Point2D Position; // [esp+D2h] [ebp-3B8h] BYREF
  int v102; // [esp+DAh] [ebp-3B0h]
  int v103; // [esp+DEh] [ebp-3ACh]
  Point2D v104; // [esp+E2h] [ebp-3A8h] BYREF
  _DWORD v105[4]; // [esp+EAh] [ebp-3A0h] BYREF
  RectangleStruct v106; // [esp+FAh] [ebp-390h]
  Point2D v107; // [esp+10Ah] [ebp-380h] BYREF
  Point2D a1a; // [esp+112h] [ebp-378h] BYREF
  Point2D v109; // [esp+11Ah] [ebp-370h] BYREF
  Point2D v110; // [esp+122h] [ebp-368h] BYREF
  wchar_t Destination[64]; // [esp+12Ah] [ebp-360h] BYREF
  RectangleStruct v112; // [esp+1AAh] [ebp-2E0h] BYREF
  RectangleStruct pOutBuffer; // [esp+1BAh] [ebp-2D0h] BYREF
  RectangleStruct v114; // [esp+1CAh] [ebp-2C0h] BYREF
  RectangleStruct v115; // [esp+1DAh] [ebp-2B0h] BYREF
  wchar_t v116[20]; // [esp+1EAh] [ebp-2A0h] BYREF
  wchar_t v117[20]; // [esp+212h] [ebp-278h] BYREF
  wchar_t Format[20]; // [esp+23Ah] [ebp-250h] BYREF
  wchar_t v119[20]; // [esp+262h] [ebp-228h] BYREF
  wchar_t Buffer[256]; // [esp+28Ah] [ebp-200h] BYREF

  v78 = a1;
  result = a1->AllowedToDraw;
  if ( result )
  {
    if ( a1->NeedsRedraw || (result = a3) != 0 )
    {
      a1->NeedsRedraw = 0;
      sidebar_draw_isdirty_B0B518 = 1;
      a4.X = 0;
      a4.Y = 0;
      a4.Width = DSurface_Sidebar_887300->vt->Surface__GetWidth_411540((Surface *)DSurface_Sidebar_887300);
      v5 = DSurface_Sidebar_887300->vt->Surface__GetHeight_411550((Surface *)DSurface_Sidebar_887300);
      v6 = CurrentPlayer_A83D4C;
      v7 = Observer_AC1198;
      a4.Height = v5;
      CameoCount = 0;
      v74 = 0;
      if ( CurrentPlayer_A83D4C == Observer_AC1198 )
        CameoCount = a1->CameoCount;
      v9 = a1->CameoCount;
      if ( CameoCount < v9 )
      {
        v10 = &a1->Cameos[0].unknown_10 + 12 * CameoCount + CameoCount;
        while ( *v10 != 1 && *v10 != 2 )
        {
          ++CameoCount;
          v10 += 13;
          if ( CameoCount >= v9 )
            goto LABEL_13;
        }
        v74 = 1;
      }
LABEL_13:
      v11 = 26;
      if ( ScenarioClass_A8B230->PlayerSideIndex )
        v11 = 18;
      v79 = 0;
      v12 = (a1->unknown_3F != 0)
          + (DSurface_Rect_SidebarBounds_886F90.Height - dword_B0B4F8 - v11 + DSurface_Rect_SidebarBounds_886F90.Y - 7)
          / 50;
      v86 = (HouseClass *)v12;
      if ( CurrentPlayer_A83D4C != Observer_AC1198 )
      {
        if ( v12 > 0 )
        {
          do
          {
            for ( i = 0; i < 2; ++i )
            {
              v73 = a3;
              TopRowIndex = v78->TopRowIndex;
              v83 = 0;
              SHP = 0;
              v80 = 0;
              BufferCount = 0;
              x = v78->Location.x;
              v84 = i + 2 * (TopRowIndex + v79);
              v15 = x + i * dword_B0B4FC - DSurface_Rect_SidebarBounds_886F90.X;
              v16 = 0;
              v68 = 0;
              v17 = v79 * dword_B0B500 + v78->Location.y + 1;
              Index = v78->Index;
              Has_Completed = 0;
              v88 = v17;
              v19 = 56 * (i + 2 * v79 + 60 * Index) + 11566720;
              if ( v78->unknown_3F )
              {
                v17 += v78->unknown_4C - dword_B0B500;
                v88 = v17;
              }
              v20 = v78->CameoCount;
              UIName = 0;
              v82 = 0;
              if ( v84 >= v20 )
              {
                LOBYTE(v68) = 0;
              }
              else
              {
                if ( !*(_BYTE *)(v19 + 52) || (HIBYTE(v68) = 1, byte_A8ED9C) )
                  HIBYTE(v68) = 0;
                ItemType = v78->Cameos[v84].ItemType;
                v22 = (char *)v78 + 52 * v84;
                v81 = v22;
                if ( ItemType == Special )
                {
                  v30 = *((_DWORD *)v22 + 22);
                  v31 = *((_DWORD *)vec_SuperWeaponTypeClass_A8E330.vec.vector + v30);
                  UIName = *(wchar_t **)(v31 + 96);
                  if ( v30 >= vec_SuperWeaponTypeClass_A8E330.Count )
                    SHP = 0;
                  else
                    SHP = *(SHPStruct **)(v31 + 184);
                  LOBYTE(v68) = 1;
                  Has_Completed = !SuperClass::ShouldDrawProgress_6CDE90(CurrentPlayer_A83D4C->Supers.vec.Items[v30]);
                  BufferCount = SuperClass_NameReadiness_6CC2B0(CurrentPlayer_A83D4C->Supers.vec.Items[v30]);
                  v83 = SuperClass_AnimStage_6CBEE0(CurrentPlayer_A83D4C->Supers.vec.Items[v30]);
                  v16 = 0;
                  v73 = 1;
                }
                else
                {
                  v23 = TechnoTypeClass_GetByTypeAndIndex_48DCD0(ItemType, *((_DWORD *)v22 + 22));
                  v82 = v23;
                  if ( v23 )
                  {
                    UIName = v23->UIName;
                    v72 = 0;
                    if ( ((int (__thiscall *)(TechnoTypeClass *))v23->vtb->anonymous_0)(v23) == 7 && v74
                      || !v23->vtb->ObjectTypeClass__FindFactory_5F7900(
                            (BuildingTypeClass *)v23,
                            1,
                            1,
                            1,
                            CurrentPlayer_A83D4C)
                      || (v80 = TechnoTypeClass_GetByTypeAndIndex_48DCD0(
                                  *((AbstractType *)v81 + 23),
                                  *((_DWORD *)v81 + 22)),
                          v69 = tech_buildtype_4F7870(CurrentPlayer_A83D4C, v80, 0, 0) == -1,
                          v24 = sub_50B370(CurrentPlayer_A83D4C, (int *)v80),
                          v69)
                      || v24 )
                    {
                      v72 = 1;
                    }
                    SHP = v23->vtb->TechnoTypeClass__GetCameo_712040(v23);
                    v80 = (TechnoTypeClass *)*((_DWORD *)v81 + 25);
                    if ( !v80 )
                    {
                      v16 = v72;
                      LOBYTE(v68) = 0;
                      v27 = &v78->Cameos[0].unknown_10 + 12 * v84 + v84;
                      switch ( *v27 )
                      {
                        case 0:
                          if ( a3 )
                            goto LABEL_59;
                          continue;
                        case 1:
                          Progress = 0;
                          v73 = 1;
                          LOBYTE(v68) = 1;
                          if ( *v27 != 1 )
                            goto LABEL_45;
                          v26 = *((_DWORD *)v81 + 27);
                          goto LABEL_43;
                        case 2:
                          v28 = 0;
                          LOBYTE(v68) = 1;
                          Has_Completed = 0;
                          v16 = 1;
                          if ( *v27 == 1 )
                          {
                            v29 = *((_DWORD *)v81 + 27);
                            if ( v29 > 0 )
                              v28 = v29 / 2;
                          }
                          v83 = v28;
                          goto LABEL_59;
                        case 3:
                          LOBYTE(v68) = 1;
                          Has_Completed = 1;
                          v16 = 0;
                          goto LABEL_59;
                        default:
                          goto LABEL_59;
                      }
                    }
                    LOBYTE(v68) = 1;
                    Has_Completed = FactoryClass::Has_Completed((FactoryClass *)v80);
                    if ( Has_Completed )
                      BufferCount = (const wchar_t *)StringTable_LoadString_734E60(
                                                       (wchar_t *)&aNuke.VoiceDie,
                                                       (int)aDRa2mdpostSide,
                                                       3411);
                    else
                      v73 = 1;
                    Progress = 0;
                    if ( *((_DWORD *)v81 + 25) )
                      Progress = FactoryClass_GetProgress();
                    if ( *(&v78->Cameos[0].unknown_10 + 12 * v84 + v84) == 1 )
                    {
                      v26 = *((_DWORD *)v81 + 27);
LABEL_43:
                      if ( v26 > Progress )
                        Progress = (v26 + Progress) / 2;
                    }
LABEL_45:
                    v83 = Progress;
                  }
                  v16 = 0;
                }
              }
LABEL_59:
              Bounds.X = 0;
              Bounds.Y = 0;
              Bounds.Width = v15 + 60;
              Bounds.Height = v17 + 48;
              if ( v73 )
              {
                if ( SHP )
                {
                  Position.x = v15;
                  Position.y = v17;
                  CC_Draw_Shape_4AED70(
                    DSurface_Sidebar_887300,
                    FileSystem_ConvertClass_CAMEO_PAL_87F6B0,
                    (SHPReference *)SHP,
                    0,
                    &Position,
                    &Bounds,
                    1024,
                    0,
                    0,
                    0,
                    1000,
                    0,
                    0,
                    0,
                    0,
                    0);
                }
                if ( HIBYTE(v68) )
                {
                  v105[0] = v15;
                  v105[1] = v17;
                  v105[2] = 60;
                  v105[3] = 48;
                  BYTE2(a5.x) = BYTE2(CCToolTip_ToolTipTextColor);
                  vt = DSurface_Sidebar_887300->vt;
                  LOWORD(a5.x) = CCToolTip_ToolTipTextColor;
                  vt->XSurface__DrawRectEx_7BADC0(
                    (Surface *)DSurface_Sidebar_887300,
                    &Bounds,
                    (RectangleStruct *)v105,
                    ((unsigned __int8)CCToolTip_ToolTipTextColor >> RedShiftRight_8A0DD4 << RedShiftLeft_8A0DD0)
                  | (BYTE1(CCToolTip_ToolTipTextColor) >> GreenShiftRight_8A0DE4 << GreenShiftLeft_8A0DE0)
                  | (BYTE2(CCToolTip_ToolTipTextColor) >> BlueShiftRight_8A0DDC << BlueShiftLeft_8A0DD8));
                }
                if ( v16 )
                {
                  v104.x = v15;
                  v104.y = v17;
                  CC_Draw_Shape_4AED70(
                    DSurface_Sidebar_887300,
                    FileSystem_ConvertClass_SIDEBAR_PAL_87F6CC,
                    (SHPReference *)dword_B07BC0,
                    0,
                    &v104,
                    &Bounds,
                    1025,
                    0,
                    0,
                    0,
                    1000,
                    0,
                    0,
                    0,
                    0,
                    0);
                }
                if ( v78->Cameos[v84].FlashEndFrame > (signed int)CurrentFrame_A8ED84
                  && (int)CurrentFrame_A8ED84 % 16 > 8 )
                {
                  v100.x = v15;
                  v100.y = v17;
                  CC_Draw_Shape_4AED70(
                    DSurface_Sidebar_887300,
                    FileSystem_ConvertClass_SIDEBAR_PAL_87F6CC,
                    (SHPReference *)dword_B07BC0,
                    0,
                    &v100,
                    &Bounds,
                    1028,
                    0,
                    0,
                    0,
                    1000,
                    0,
                    0,
                    0,
                    0,
                    0);
                }
                if ( UIName )
                {
                  v103 = v17 + 36;
                  v102 = v15;
                  sub_6AC480(&a4, 60);
                }
                v70 = 0;
                v33 = ((unsigned __int8)CCToolTip_ToolTipTextColor >> RedShiftRight_8A0DD4 << RedShiftLeft_8A0DD0)
                    | (BYTE1(CCToolTip_ToolTipTextColor) >> GreenShiftRight_8A0DE4 << GreenShiftLeft_8A0DE0)
                    | (BYTE2(CCToolTip_ToolTipTextColor) >> BlueShiftRight_8A0DDC << BlueShiftLeft_8A0DD8);
                if ( v82 )
                {
                  Naval = v82->Naval;
                  v34 = ((int (__thiscall *)(TechnoTypeClass *))v82->vtb->anonymous_0)(v82);
                  PrimaryFactory_500510 = HouseClass::GetPrimaryFactory_500510(
                                            CurrentPlayer_A83D4C,
                                            v34,
                                            Naval,
                                            DontCare);
                  if ( PrimaryFactory_500510 )
                  {
                    v35 = CountTotal(v82);
                    v36 = (const wchar_t *)v35;
                    if ( v35 > 1
                      || v35 > 0
                      && ((Object = PrimaryFactory_500510->Object) == 0
                       || Object->vtb->TechnoClass__GetTechnoType_6F3270(Object)
                       && PrimaryFactory_500510->Object->vtb->TechnoClass__GetTechnoType_6F3270(PrimaryFactory_500510->Object) != v82) )
                    {
                      swprintf(Buffer, (const size_t)&aNuke.VoiceSelectEnslaved.Dvec.vec.Capacity, v36);
                      v99[1] = v17 + 1;
                      v63.y = v17 + 1;
                      v63.x = v15 + 60;
                      v99[0] = v15 + 60;
                      v106 = *Drawing_GetTextDimensions_4A59E0(&pOutBuffer, Buffer, v63, 0x242u, 2, 1);
                      sub_621B80(0, 175);
                      Fancy_Text_Print_Wide_4A60E0(
                        (int)&v107,
                        (size_t)&aNuke.VoiceSelectEnslaved.Dvec.vec.Capacity,
                        (int)DSurface_Sidebar_887300,
                        (int)&a4,
                        (int)v99,
                        v33,
                        0,
                        578,
                        (wchar_t)v36);
                      v70 = 1;
                    }
                  }
                }
                if ( (_BYTE)v68 )
                {
                  if ( BufferCount )
                  {
                    v98.y = v17 + 1;
                    v64.y = v17 + 1;
                    v64.x = v15 + 30;
                    v98.x = v15 + 30;
                    v95 = *Drawing_GetTextDimensions_4A59E0(&v114, BufferCount, v64, 0x142u, 2, 1);
                    sub_621B80(0, 175);
                    Fancy_Text_Print_Wide_4A60E0(
                      (int)&v109,
                      (size_t)BufferCount,
                      (int)DSurface_Sidebar_887300,
                      (int)&a4,
                      (int)&v98,
                      v33,
                      0,
                      322,
                      a2);
                  }
                  if ( !Has_Completed )
                  {
                    if ( (v91.x = v15,
                          v91.y = v17,
                          CC_Draw_Shape_4AED70(
                            DSurface_Sidebar_887300,
                            FileSystem_ConvertClass_SIDEBAR_PAL_87F6CC,
                            (SHPReference *)dword_B0B484,
                            v83 + 1,
                            &v91,
                            &Bounds,
                            1028,
                            0,
                            0,
                            0,
                            1000,
                            0,
                            0,
                            0,
                            0,
                            0),
                          v80)
                      && (!*(_DWORD *)&v80->ID[20] || v80->Name[12])
                      || *(&v78->Cameos[0].unknown_10 + 12 * v84 + v84) == 2 )
                    {
                      String_734E60 = (const wchar_t *)StringTable_LoadString_734E60(
                                                         aTxtHold,
                                                         (int)aDRa2mdpostSide,
                                                         3636);
                      v65.y = v88 + 1;
                      if ( v70 )
                      {
                        v65.x = v15 + 2;
                        v92.x = v15 + 2;
                        v92.y = v88 + 1;
                        v96 = *Drawing_GetTextDimensions_4A59E0(&v115, String_734E60, v65, 0x42u, 2, 1);
                        sub_621B80(0, 175);
                        Fancy_Text_Print_Wide_4A60E0(
                          (int)&v110,
                          (size_t)String_734E60,
                          (int)DSurface_Sidebar_887300,
                          (int)&a4,
                          (int)&v92,
                          v33,
                          0,
                          66,
                          a2);
                      }
                      else
                      {
                        v65.x = v15 + 30;
                        v93.x = v15 + 30;
                        v93.y = v88 + 1;
                        v94 = *Drawing_GetTextDimensions_4A59E0(&v112, String_734E60, v65, 0x142u, 2, 1);
                        sub_621B80(0, 175);
                        Fancy_Text_Print_Wide_4A60E0(
                          (int)&a1a,
                          (size_t)String_734E60,
                          (int)DSurface_Sidebar_887300,
                          (int)&a4,
                          (int)&v93,
                          v33,
                          0,
                          322,
                          a2);
                      }
                    }
                  }
                }
              }
            }
            ++v79;
          }
          while ( v79 < (int)v86 );
          v7 = Observer_AC1198;
          v6 = CurrentPlayer_A83D4C;
        }
        if ( v6 != v7 )
        {
LABEL_135:
          result = (char)v78;
          v78->unknown_50 = v78->unknown_4C;
          return result;
        }
        v12 = (int)v86;
      }
      v79 = 0;
      v80 = (TechnoTypeClass *)(v12 / 2);
      if ( v12 / 2 > 0 )
      {
        while ( 1 )
        {
          v39 = v78->TopRowIndex + v79;
          v40 = v78->Location.y + 2 * v79 * dword_B0B500 + 1;
          v41 = v78->Location.x - DSurface_Rect_SidebarBounds_886F90.X;
          if ( v39 >= MapClass_87F7E8.DiplomacyNumHouses )
            goto LABEL_135;
          v86 = MapClass_87F7E8.DiplomacyHouses[v39];
          v42 = v86;
          if ( !v86 )
            goto LABEL_135;
          Type = v86->Type;
          if ( Type )
          {
            SideIndex = Type->SideIndex;
            if ( !SideIndex )
            {
              v46 = (SHPReference *)dword_B0B490;
LABEL_105:
              if ( v46 )
              {
                v47 = v41 + v46->st.Width;
                v94.X = 0;
                v94.Width = v47;
                v94.Y = 0;
                v48 = v40 + v46->st.Height;
                v93.x = v41;
                v94.Height = v48;
                v93.y = v40;
                CC_Draw_Shape_4AED70(
                  DSurface_Sidebar_887300,
                  GRFXTXT_Convert_87F6D0,
                  v46,
                  0,
                  &v93,
                  &v94,
                  1024,
                  0,
                  0,
                  0,
                  1000,
                  0,
                  0,
                  0,
                  0,
                  0);
              }
              goto LABEL_107;
            }
            v45 = SideIndex - 1;
            if ( !v45 )
            {
              v46 = (SHPReference *)dword_B0B494;
              goto LABEL_105;
            }
            if ( v45 == 1 )
            {
              v46 = (SHPReference *)dword_B0B498;
              goto LABEL_105;
            }
          }
LABEL_107:
          v49 = v42->Type;
          if ( v49 )
          {
            ArrayIndex2 = v49->CountryIndex;
            switch ( ArrayIndex2 )
            {
              case -3:
                v51 = (SHPReference *)dword_B0B4A0;
                goto LABEL_121;
              case -2:
                v51 = (SHPReference *)dword_B0B49C;
                goto LABEL_121;
              case 0:
                v51 = (SHPReference *)dword_B0B4A4;
                goto LABEL_121;
              case 1:
                v51 = (SHPReference *)dword_B0B4A8;
                goto LABEL_121;
              case 2:
                v51 = (SHPReference *)dword_B0B4AC;
                goto LABEL_121;
              case 3:
                v51 = (SHPReference *)dword_B0B4B0;
                goto LABEL_121;
              case 4:
                v51 = (SHPReference *)dword_B0B4B4;
                goto LABEL_121;
              case 5:
                v51 = (SHPReference *)dword_B0B4B8;
                goto LABEL_121;
              case 6:
                v51 = (SHPReference *)dword_B0B4BC;
                goto LABEL_121;
              case 7:
                v51 = (SHPReference *)dword_B0B4C0;
                goto LABEL_121;
              case 8:
                v51 = (SHPReference *)dword_B0B4C4;
                goto LABEL_121;
              case 9:
                v51 = (SHPReference *)dword_B0B4C8;
LABEL_121:
                if ( v51 )
                {
                  if ( ArrayIndex2 == 9 )
                  {
                    v52 = (ConvertClass *)sub_72F4D0();
                    v53 = v51->st.Width + v41 + 70;
                    v96.X = 0;
                    v96.Width = v53;
                    v96.Y = 0;
                    v96.Height = v51->st.Height + v40 + 70;
                    v92.x = v41 + 70;
                    v92.y = v40 + 70;
                    CC_Draw_Shape_4AED70(
                      DSurface_Sidebar_887300,
                      v52,
                      v51,
                      0,
                      &v92,
                      &v96,
                      1024,
                      0,
                      0,
                      0,
                      1000,
                      0,
                      0,
                      0,
                      0,
                      0);
                  }
                  else
                  {
                    v54 = v51->st.Width + v41 + 70;
                    v95.X = 0;
                    Height = v51->st.Height;
                    v95.Width = v54;
                    v95.Y = 0;
                    v95.Height = Height + v40 + 70;
                    v91.x = v41 + 70;
                    v91.y = v40 + 70;
                    CC_Draw_Shape_4AED70(
                      DSurface_Sidebar_887300,
                      GRFXTXT_Convert_87F6D0,
                      v51,
                      0,
                      &v91,
                      &v95,
                      1024,
                      0,
                      0,
                      0,
                      1000,
                      0,
                      0,
                      0,
                      0,
                      0);
                  }
                }
                break;
              default:
                break;
            }
          }
          wcscpy(word_B078B0, (const wchar_t *)v86->UIName);
          if ( SessionClass_Instance_A8B238.GameMode == Internet )
            _itow(MapClass_87F7E8.unknown_544C[v39], Format, 10);
          _itow(MapClass_87F7E8.DiplomacyKills[v39], v116, 10);
          _itow(MapClass_87F7E8.DiplomacyOwned[v39], v119, 10);
          _itow(MapClass_87F7E8.DiplomacyPowerDrain[v39], v117, 10);
          v56 = MapClass_87F7E8.DiplomacyColors[v39];
          if ( v78->unknown_3F )
            v40 += v78->unknown_4C - dword_B0B500;
          a5.x = v41 + 8;
          a5.y = v40 + 4;
          Drawing_PrintUnicode_4A61C0(
            &a1a,
            (const char *)word_B078B0,
            (Surface *)DSurface_Sidebar_887300,
            &a4,
            &a5,
            v56,
            0,
            FullShadow|Point8);
          if ( SessionClass_Instance_A8B238.GameMode == Internet )
          {
            v57 = MapClass_87F7E8.unknown_544C[v39];
            a5.y += 17;
            if ( v57 >= 1 )
            {
              v59 = StringTable_LoadString_734E60(aK, (int)aDRa2mdpostSide, 3785);
              swprintf(Destination, v59, Format);
            }
            else
            {
              v58 = (const wchar_t *)StringTable_LoadString_734E60(aTxtUnranked, (int)aDRa2mdpostSide, 3781);
              wcscpy(Destination, v58);
            }
            Drawing_PrintUnicode_4A61C0(
              &v110,
              (const char *)Destination,
              (Surface *)DSurface_Sidebar_887300,
              &a4,
              &a5,
              v56,
              0,
              FullShadow|Point8);
          }
          a5.y += 17;
          v60 = StringTable_LoadString_734E60(aGuiObskills, (int)aDRa2mdpostSide, 3807);
          swprintf(Destination, v60, v116);
          Drawing_PrintUnicode_4A61C0(
            &v109,
            (const char *)Destination,
            (Surface *)DSurface_Sidebar_887300,
            &a4,
            &a5,
            v56,
            0,
            FullShadow|Point8);
          a5.y += 17;
          v61 = StringTable_LoadString_734E60(aGuiObsunits, (int)aDRa2mdpostSide, 3822);
          swprintf(Destination, v61, v119);
          Drawing_PrintUnicode_4A61C0(
            &v107,
            (const char *)Destination,
            (Surface *)DSurface_Sidebar_887300,
            &a4,
            &a5,
            v56,
            0,
            FullShadow|Point8);
          a5.y += 17;
          v62 = StringTable_LoadString_734E60(aGuiObscredits, (int)aDRa2mdpostSide, 3835);
          swprintf(Destination, v62, v117);
          Drawing_PrintUnicode_4A61C0(
            &v98,
            (const char *)Destination,
            (Surface *)DSurface_Sidebar_887300,
            &a4,
            &a5,
            v56,
            0,
            FullShadow|Point8);
          if ( ++v79 >= (int)v80 )
            goto LABEL_135;
        }
      }
      goto LABEL_135;
    }
  }
  return result;
}