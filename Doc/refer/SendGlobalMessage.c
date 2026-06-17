void __thiscall Message_Input_55E420(_DWORD *Input)
{
  signed int v2; // eax
  int v3; // edx
  unsigned __int16 *v4; // esi
  signed int v5; // edi
  int v6; // edx
  int v7; // eax
  int v8; // esi
  int v9; // eax
  bool v10; // bl
  size_t String_734E60; // eax
  size_t v12; // eax
  unsigned int color_enum_69A310; // eax
  int v14; // eax
  const wchar_t *v15; // eax
  int v16; // esi
  int v17; // eax
  int v18; // edi
  const wchar_t *Edit_Buf_5D4400; // eax
  const wchar_t *v20; // eax
  wchar_t *v21; // eax
  int v22; // eax
  int v23; // ebx
  const wchar_t *v24; // eax
  wchar_t *v25; // esi
  wchar_t *v26; // edi
  wchar_t v27; // cx
  int v28; // esi
  size_t v29; // eax
  wchar_t *v30; // eax
  int v31; // eax
  char v32; // bl
  const wchar_t *v33; // eax
  const wchar_t *v34; // eax
  size_t v35; // eax
  wchar_t *v36; // esi
  NodeNameType **Items; // edi
  int v38; // eax
  const wchar_t *v39; // eax
  int v40; // esi
  size_t v41; // eax
  const wchar_t *v42; // eax
  int count; // edi
  int v44; // edx
  int v45; // esi
  HouseClass *v46; // eax
  int ColorScheme_4E12D0; // esi
  wchar_t *v48; // eax
  HouseClass *v49; // esi
  int v50; // ebp
  int v51; // edi
  const wchar_t *v52; // ebx
  int v53; // esi
  wchar_t *connection_player_name_540FC0; // eax
  IPXAddressClass *connection_address_541000; // eax
  wchar_t *v56; // esi
  unsigned int v57; // eax
  int v58; // esi
  const wchar_t *v59; // eax
  wchar_t *v60; // [esp-14h] [ebp-8B4h]
  wchar_t *v61; // [esp-14h] [ebp-8B4h]
  wchar_t *v62; // [esp-14h] [ebp-8B4h]
  wchar_t *v63; // [esp-14h] [ebp-8B4h]
  unsigned int v64; // [esp-10h] [ebp-8B0h]
  unsigned int v65; // [esp-10h] [ebp-8B0h]
  unsigned int v66; // [esp-10h] [ebp-8B0h]
  unsigned int v67; // [esp-10h] [ebp-8B0h]
  unsigned int v68; // [esp-10h] [ebp-8B0h]
  unsigned int v69; // [esp-10h] [ebp-8B0h]
  int v70; // [esp-8h] [ebp-8A8h]
  int v71; // [esp-8h] [ebp-8A8h]
  int v72; // [esp-8h] [ebp-8A8h]
  int v73; // [esp-8h] [ebp-8A8h]
  int v74; // [esp-8h] [ebp-8A8h]
  int v75; // [esp-8h] [ebp-8A8h]
  int v76; // [esp-8h] [ebp-8A8h]
  int v77; // [esp-8h] [ebp-8A8h]
  __int16 *UIName; // [esp-4h] [ebp-8A4h]
  const wchar_t *v79; // [esp-4h] [ebp-8A4h]
  __int16 *v80; // [esp-4h] [ebp-8A4h]
  __int16 *v81; // [esp-4h] [ebp-8A4h]
  const wchar_t *v82; // [esp+0h] [ebp-8A0h]
  int rc; // [esp+10h] [ebp-890h] BYREF
  int v84; // [esp+14h] [ebp-88Ch]
  int v85; // [esp+18h] [ebp-888h]
  wchar_t v86[16]; // [esp+1Ch] [ebp-884h] BYREF
  wchar_t String[64]; // [esp+3Ch] [ebp-864h] BYREF
  wchar_t player_name[64]; // [esp+BCh] [ebp-7E4h] BYREF
  wchar_t Destination[161]; // [esp+13Ch] [ebp-764h] BYREF
  __int16 v90; // [esp+27Eh] [ebp-622h]
  wchar_t String1[224]; // [esp+280h] [ebp-620h] BYREF
  wchar_t v92[112]; // [esp+440h] [ebp-460h] BYREF
  wchar_t Source[224]; // [esp+520h] [ebp-380h] BYREF
  wchar_t Buffer[224]; // [esp+6E0h] [ebp-1C0h] BYREF

  v2 = wcslen(&IME_buffer_B730EC);
  if ( v2 )
  {
    rc = 0;
    if ( v2 > 0 )
    {
      v4 = &IME_buffer_B730EC;
      v5 = v2;
      do
      {
        Session_Messages_Input_5D4510((int)&SessionClass_Instance_A8B238.Messages, v3, &rc, *v4++);
        --v5;
      }
      while ( v5 );
    }
    IME_buffer_B730EC = 0;
  }
  v6 = *Input;
  if ( *Input )
  {
    if ( SessionClass_Instance_A8B238.GameMode
      && SessionClass_Instance_A8B238.GameMode != Skirmish
      && !SessionClass_Instance_A8B238.Messages.IsEdit )
    {
      if ( (v6 & 0x8000) != 0 )
        return;
      v7 = (unsigned __int8)sub_54F450(*Input);
      v8 = v7;
      if ( v7 == 13 || v7 == 92 || v7 == 8 )
      {
        Buffer[0] = 0;
        v9 = sub_431170(CurrentPlayer_A83D4C->ArrayIndex);
        v10 = v9 != 0;
        if ( v9 )
        {
          String_734E60 = StringTable_LoadString_734E60(aTxtBeaconMessa, (int)aDRa2mdpostMain, 1863);
          swprintf(Buffer, String_734E60, v82);
        }
        else
        {
          UIName = CurrentPlayer_A83D4C->UIName;
          v12 = StringTable_LoadString_734E60(aTxtFrom, (int)aDRa2mdpostMain, 1865);
          swprintf(Buffer, v12, (const wchar_t *const)UIName);
          switch ( v8 )
          {
            case 13:
              rc_ABCE18 = 1;                    // VK_RETURN
              break;
            case 92:                            // VK_RWIN
              rc_ABCE18 = 3;
              break;
            case 8:
              rc_ABCE18 = 2;                    // VK_BACK
              break;
          }
        }
        *Input = 0;
        color_enum_69A310 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
        v14 = MessageListClass_AddEdit_5D4210(color_enum_69A310, 16454, Buffer, 0, v10 ? 10000 : -1);
        if ( v10 )
        {
          if ( v14 )
            *(_BYTE *)(v14 + 64) = 1;
          SessionClass_Instance_A8B238.Messages.EnableOverflow = 0;
          v15 = (const wchar_t *)fuck_buggy_mbstowcs_735120(asc_818D7C);
          wcscpy(v86, v15);
          beacon_text_431450(v86, -1, -1, 0);
        }
        else
        {
          SessionClass_Instance_A8B238.Messages.EnableOverflow = 1;
        }
        GScreenClass::MarkNeedsRedraw_4F42F0((GScreenClass *)&MapClass_87F7E8, 0);
        IME_buffer_B730EC = 0;
      }
    }
    v16 = *Input;
    v17 = Session_Messages_Input_5D4510((int)&SessionClass_Instance_A8B238.Messages, v6, Input, 0);
    v18 = v17;
    rc = v17;
    if ( v17 == 1 )
    {
      if ( SessionClass_Instance_A8B238.GameMode == Campaign )
        goto LABEL_31;
    }
    else
    {
      if ( v17 != 2 || SessionClass_Instance_A8B238.GameMode == Campaign )
        goto LABEL_31;
      if ( v16 == 27 )
      {
        GScreenClass::MarkNeedsRedraw_4F42F0((GScreenClass *)&MapClass_87F7E8, 2);
        rc_ABCE18 = 0;
LABEL_31:
        if ( sub_431170(CurrentPlayer_A83D4C->ArrayIndex) && v16 && v18 == 1 || v18 == 2 || v18 == 4 || dword_B7356C )
        {
          if ( v16 == 27 )
          {
            beacon_text_431450(0, -1, -1, 1);
          }
          else
          {
            Edit_Buf_5D4400 = (const wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            wcsncpy(Destination, Edit_Buf_5D4400, 0xA2u);
            v90 = 0;
            wcsncat(Destination, WideCharStr, 0xA2u);
            v90 = 0;
            v20 = (const wchar_t *)fuck_buggy_mbstowcs_735120(asc_818D7C);
            wcsncat(Destination, v20, 0xA2u);
            v90 = 0;
            beacon_text_431450(Destination, -1, -1, 0);
          }
        }
        if ( v18 != 3 && v18 != 4
          || SessionClass_Instance_A8B238.GameMode == Campaign
          || SessionClass_Instance_A8B238.GameMode == Skirmish )
        {
          goto LABEL_148;
        }
        if ( SessionClass_Instance_A8B238.GameMode == 2 || SessionClass_Instance_A8B238.GameMode == 1 )
        {
          v58 = dword_A8EA74;
          *(_DWORD *)dword_A8EA74 = 104;
          fuck_buggy_wcstombs_735090(
            (_BYTE *)(v58 + 4),
            *(char **)SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items,
            -1);
          *(_BYTE *)(v58 + 24) = SessionClass_Instance_A8B238.PlayerColor;
          if ( v18 == 3 )
          {
            v59 = (const wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            wcscpy((wchar_t *)(v58 + 25), v59);
          }
          else
          {
            wcscpy((wchar_t *)(v58 + 25), (const wchar_t *)SessionClass_Instance_A8B238.Messages.OverflowBuffer);
            SessionClass_Instance_A8B238.Messages.OverflowBuffer[0] = 0;
          }
          sub_5F1BF0((int)&dword_A8EA70, dword_A8EA74, 250, 1);
          wcscpy((wchar_t *)SessionClass_Instance_A8B238.LastMessage, (const wchar_t *)(v58 + 25));
          goto LABEL_147;
        }
        if ( SessionClass_Instance_A8B238.GameMode != LAN && SessionClass_Instance_A8B238.GameMode != Internet )
        {
LABEL_147:
          GScreenClass::MarkNeedsRedraw_4F42F0((GScreenClass *)&MapClass_87F7E8, 2);
LABEL_148:
          if ( v18 == 3 )
            rc_ABCE18 = 0;
          return;
        }
        if ( sub_431170(CurrentPlayer_A83D4C->ArrayIndex) )
        {
          v21 = (wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
          beacon_text_431450(v21, -1, -1, 1);
          goto LABEL_147;
        }
        if ( SessionClass_Instance_A8B238.GameMode == Internet )
        {
          if ( v18 == 3 )
          {
            v79 = (const wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            wcscpy(String1, v79);
          }
          else
          {
            wcscpy(String1, (const wchar_t *)SessionClass_Instance_A8B238.Messages.OverflowBuffer);
          }
          v22 = sub_78A6B0(String1);
          if ( v22 )
          {
            if ( v22 != 1 )
              goto LABEL_103;
            v23 = 0;
            v24 = (const wchar_t *)StringTable_LoadString_734E60(aTxtPageCommand, (int)aDRa2mdpostMain, 2051);
            wcscpy(String, v24);
            v25 = &String1[wcslen(String)];
            if ( *v25 )
            {
              do
              {
                if ( wcsncmp(v25, (const wchar_t *)&WWMessageBox_Instance_82083C, 1u) )
                  break;
                ++v25;
              }
              while ( *v25 );
              if ( *v25 )
              {
                v26 = v86;
                do
                {
                  if ( !wcsncmp(v25, (const wchar_t *)&WWMessageBox_Instance_82083C, 1u) )
                    break;
                  if ( v23 > 10 )
                    break;
                  v27 = *v25++;
                  *v26 = v27;
                  ++v23;
                  ++v26;
                }
                while ( *v25 );
              }
            }
            for ( v86[v23] = 0; *v25; ++v25 )
            {
              if ( wcsncmp(v25, (const wchar_t *)&WWMessageBox_Instance_82083C, 1u) )
                break;
            }
            if ( wcslen(v86) )
            {
              if ( wcslen(v25) )
              {
                swprintf(
                  player_name,
                  (const size_t)&off_82A0F8,
                  *(const wchar_t *const *)SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items,
                  v86);
                v72 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
                v66 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
                Session_Messages_Add_Message_5D3BA0(
                  &SessionClass_Instance_A8B238.Messages,
                  player_name,
                  SessionClass_Instance_A8B238.PlayerColor,
                  v25,
                  v66,
                  UseGradPal|FullShadow|Point6Grad,
                  v72,
                  0);
              }
              else
              {
                v71 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
                v65 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
                v61 = (wchar_t *)StringTable_LoadString_734E60(aTxtCantSendBla, (int)aDRa2mdpostMain, 2072);
                Session_Messages_Add_Message_5D3BA0(
                  &SessionClass_Instance_A8B238.Messages,
                  0,
                  -1,
                  v61,
                  v65,
                  UseGradPal|FullShadow|Point6Grad,
                  v71,
                  0);
              }
            }
            else
            {
              v70 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
              v64 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
              v60 = (wchar_t *)StringTable_LoadString_734E60(aD_14, (int)aDRa2mdpostMain, 2067);
              Session_Messages_Add_Message_5D3BA0(
                &SessionClass_Instance_A8B238.Messages,
                0,
                -1,
                v60,
                v64,
                UseGradPal|FullShadow|Point6Grad,
                v70,
                0);
            }
            if ( rc != 4 )
              goto LABEL_103;
            v28 = Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            if ( !v28 )
              goto LABEL_103;
            v80 = CurrentPlayer_A83D4C->UIName;
            v29 = StringTable_LoadString_734E60(aTxtFrom, (int)aDRa2mdpostMain, 2091);
            swprintf(v92, v29, (const wchar_t *const)v80);
            swprintf(Source, (const size_t)&off_82A0E8, String, v86, v28);
            MessageListClass_RemoveEdit_5D4390(&SessionClass_Instance_A8B238.Messages);
            MessageListClass_AddEdit_5D4210(SessionClass_Instance_A8B238.PlayerColor, 16454, v92, 0, -1);
            v30 = (wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            if ( !v30 )
              goto LABEL_103;
            goto LABEL_102;
          }
          if ( sub_78A470(String1) )
          {
            SessionClass_Instance_A8B238.Messages.OverflowBuffer[0] = 0;
            return;
          }
          v31 = sub_78A8F0(String1);
          if ( v31 )
          {
            if ( v31 != 1 )
              goto LABEL_103;
            v32 = 0;
            v33 = (const wchar_t *)StringTable_LoadString_734E60(aD_13, (int)aDRa2mdpostMain, 2121);
            wcscpy(String, v33);
            v34 = (const wchar_t *)StringTable_LoadString_734E60(aT_2, (int)aDRa2mdpostMain, 2122);
            wcscpy(player_name, v34);
            v35 = wcslen(String);
            if ( !wcsncmp(String1, String, v35) )
            {
              v36 = &String1[wcslen(String)];
            }
            else
            {
              v36 = &String1[wcslen(player_name)];
              v32 = 1;
            }
            for ( ; *v36; ++v36 )
            {
              if ( wcsncmp(v36, (const wchar_t *)&WWMessageBox_Instance_82083C, 1u) )
                break;
            }
            Items = SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items;
            v38 = fuck_buggy_mbstowcs_735120(byte_B77E30);
            swprintf(v92, (const size_t)&off_82A0F8, (*Items)->Name, v38);
            v39 = (const wchar_t *)fuck_buggy_mbstowcs_735120(byte_B77E30);
            if ( wcslen(v39) )
            {
              if ( wcslen(v36) )
              {
                v74 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
                v68 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
                Session_Messages_Add_Message_5D3BA0(
                  &SessionClass_Instance_A8B238.Messages,
                  v92,
                  SessionClass_Instance_A8B238.PlayerColor,
                  v36,
                  v68,
                  UseGradPal|FullShadow|Point6Grad,
                  v74,
                  0);
              }
              else
              {
                v75 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
                v69 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
                v63 = (wchar_t *)StringTable_LoadString_734E60(aTxtCantSendBla, (int)aDRa2mdpostMain, 2149);
                Session_Messages_Add_Message_5D3BA0(
                  &SessionClass_Instance_A8B238.Messages,
                  0,
                  SessionClass_Instance_A8B238.PlayerColor,
                  v63,
                  v69,
                  UseGradPal|FullShadow|Point6Grad,
                  v75,
                  0);
              }
            }
            else
            {
              v73 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
              v67 = maybe_get_color_enum_69A310(SessionClass_Instance_A8B238.PlayerColor);
              v62 = (wchar_t *)StringTable_LoadString_734E60(aTxtNameNotSpec, (int)aDRa2mdpostMain, 2136);
              Session_Messages_Add_Message_5D3BA0(
                &SessionClass_Instance_A8B238.Messages,
                0,
                SessionClass_Instance_A8B238.PlayerColor,
                v62,
                v67,
                UseGradPal|FullShadow|Point6Grad,
                v73,
                0);
            }
            if ( rc != 4 )
              goto LABEL_103;
            v40 = Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            if ( !v40 )
              goto LABEL_103;
            v81 = CurrentPlayer_A83D4C->UIName;
            v41 = StringTable_LoadString_734E60(aTxtFrom, (int)aDRa2mdpostMain, 2161);
            swprintf(Destination, v41, (const wchar_t *const)v81);
            if ( v32 )
              swprintf(Source, (const size_t)&off_826960, player_name, v40);
            else
              swprintf(Source, (const size_t)&off_826960, String, v40);
            MessageListClass_RemoveEdit_5D4390(&SessionClass_Instance_A8B238.Messages);
            MessageListClass_AddEdit_5D4210(SessionClass_Instance_A8B238.PlayerColor, 16454, Destination, 0, -1);
            v30 = (wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
            if ( !v30 )
              goto LABEL_103;
LABEL_102:
            wcscat(v30, Source);
LABEL_103:
            SessionClass_Instance_A8B238.Messages.OverflowBuffer[0] = 0;
            return;
          }
        }
        SessionClass_Instance_A8B238.GPacket.Command = NET_MESSAGE;
        strcpy(
          (char *)SessionClass_Instance_A8B238.GPacket.Name,
          fuck_buggy_wcstombs_7350C0(*SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items));
        *(_DWORD *)&SessionClass_Instance_A8B238.GPacket.field_2C[228] = SessionClass_Instance_A8B238.PlayerColor;
        *(_DWORD *)&SessionClass_Instance_A8B238.GPacket.field_2C[232] = Compute_Name_CRC_5DAC00((wchar_t *)SessionClass_Instance_A8B238.GameName);
        SessionClass_Instance_A8B238.GPacket.field_2C[3] = 0;
        if ( rc == 3 )
        {
          v42 = (const wchar_t *)Session_Messages_Get_Edit_Buf_5D4400(&SessionClass_Instance_A8B238.Messages);
          wcscpy((wchar_t *)&SessionClass_Instance_A8B238.GPacket.field_2C[4], v42);
        }
        else if ( rc == 4 )
        {
          wcscpy(
            (wchar_t *)&SessionClass_Instance_A8B238.GPacket.field_2C[4],
            (const wchar_t *)SessionClass_Instance_A8B238.Messages.OverflowBuffer);
          SessionClass_Instance_A8B238.Messages.OverflowBuffer[0] = 0;
        }
        count = vec_HouseClass_A80228.count;
        v85 = vec_HouseClass_A80228.count;
        if ( SessionClass_Instance_A8B238.MultiplayerObserver )
        {
          v44 = rc_ABCE18;
          if ( rc_ABCE18 == 1 || rc_ABCE18 == 2 )
          {
            v45 = 0;
            if ( vec_HouseClass_A80228.count > 0 )
            {
              while ( 1 )
              {
                v46 = vec_HouseClass_A80228.vec.Items[v45];
                if ( v46
                  && v46->Type->Multiplay
                  && (!SessionClass_Instance_A8B238.SomeMask[v45] && v44 == 1
                   || !HouseClass::IsAlliedWith_4F9A50(
                         vec_HouseClass_A80228.vec.Items[(*SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items)->HouseIndex],
                         vec_HouseClass_A80228.vec.Items[v45])
                   && rc_ABCE18 == 2) )
                {
                  break;
                }
                if ( ++v45 >= count )
                  goto LABEL_123;
                v44 = rc_ABCE18;
              }
              ColorScheme_4E12D0 = GadgetClass::GetColorScheme_4E12D0();
              v76 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
              v48 = (wchar_t *)StringTable_LoadString_734E60(aTxtObserverCan, (int)aDRa2mdpostMain, 2234);
              Session_Messages_Add_Message_5D3BA0(
                &SessionClass_Instance_A8B238.Messages,
                0,
                0,
                v48,
                ColorScheme_4E12D0,
                NoShadow|Point6Grad,
                v76,
                1);
              rc_ABCE18 = 0;
            }
          }
        }
LABEL_123:
        v84 = 0;
        if ( count > 0 )
        {
          do
          {
            v49 = vec_HouseClass_A80228.vec.Items[v84];
            if ( v49
              && v49->Type->Multiplay
              && (SessionClass_Instance_A8B238.SomeMask[v84] && rc_ABCE18 == 1
               || HouseClass::IsAlliedWith_4F9A50(
                    vec_HouseClass_A80228.vec.Items[(*SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items)->HouseIndex],
                    vec_HouseClass_A80228.vec.Items[v84])
               && rc_ABCE18 == 2
               || rc_ABCE18 == 3) )
            {
              v50 = IPXManagerClass::Num_Connections_540F90((IPXConnClass *)&IPXMgr_Instance_A8E9C0);
              v51 = 0;
              if ( v50 > 0 )
              {
                v52 = (const wchar_t *)v49->UIName;
                do
                {
                  v53 = IPXManagerClass::Connection_ID_540FA0(&IPXMgr_Instance_A8E9C0, v51);
                  connection_player_name_540FC0 = get_connection_player_name_540FC0(
                                                    (IPXConnClass *)&IPXMgr_Instance_A8E9C0,
                                                    v53);
                  if ( !wcscmp(v52, connection_player_name_540FC0) )
                  {
                    connection_address_541000 = (IPXAddressClass *)get_connection_address_541000(
                                                                     (IPXConnClass *)&IPXMgr_Instance_A8E9C0,
                                                                     v53);
                    if ( SessionClass_Instance_A8B238.gap2968[0] )
                    {
                      SessionClass_Instance_A8B238.GPacket.field_2C[3] |= sub_663D60(
                                                                            &OwnerTalkClass::ConnectionListStruct_B052D0,
                                                                            v53);
                    }
                    else
                    {
                      IPXManagerClass::Send_Global_Message_5410F0(
                        &IPXMgr_Instance_A8E9C0,
                        &SessionClass_Instance_A8B238.GPacket,
                        455,
                        1,
                        connection_address_541000,
                        0,
                        0);
                      IPXManagerClass::Service_541820(&IPXMgr_Instance_A8E9C0);
                    }
                  }
                  ++v51;
                }
                while ( v51 < v50 );
              }
            }
            ++v84;
          }
          while ( v84 < v85 );
        }
        if ( SessionClass_Instance_A8B238.gap2968[0] && SessionClass_Instance_A8B238.GPacket.field_2C[3] )
        {
          WWDebug_Printf("Sending chat, mask = %d\n", (unsigned __int8)SessionClass_Instance_A8B238.GPacket.field_2C[3]);
          IPXManagerClass::Send_Global_Message_5410F0(
            &IPXMgr_Instance_A8E9C0,
            &SessionClass_Instance_A8B238.GPacket,
            455,
            1,
            (IPXAddressClass *)&(*SessionClass_Instance_A8B238.Chat(StartSpots).vec.Items)->Address,
            0,
            0);
          IPXManagerClass::Service_541820(&IPXMgr_Instance_A8E9C0);
        }
        v56 = (wchar_t *)fuck_buggy_mbstowcs_735120(SessionClass_Instance_A8B238.GPacket.Name);
        v77 = double2int_7C5F00(RuleClass::Instance_8871E0->MessageDelay * 900.0);
        v57 = maybe_get_color_enum_69A310(*(unsigned int *)&SessionClass_Instance_A8B238.GPacket.field_2C[228]);
        Session_Messages_Add_Message_5D3BA0(
          &SessionClass_Instance_A8B238.Messages,
          v56,
          *(int *)&SessionClass_Instance_A8B238.GPacket.field_2C[228],
          (wchar_t *)&SessionClass_Instance_A8B238.GPacket.field_2C[4],
          v57,
          UseGradPal|FullShadow|Point6Grad,
          v77,
          0);
        wcscpy(
          (wchar_t *)SessionClass_Instance_A8B238.LastMessage,
          (const wchar_t *)&SessionClass_Instance_A8B238.GPacket.field_2C[4]);
        v18 = rc;
        goto LABEL_147;
      }
    }
    GScreenClass::MarkNeedsRedraw_4F42F0((GScreenClass *)&MapClass_87F7E8, 0);
    goto LABEL_31;
  }
}