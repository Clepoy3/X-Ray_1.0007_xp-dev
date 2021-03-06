#include "stdafx.h"
#include <dinput.h>
#include "../HUDManager.h"
#include "UICustomEdit.h"
#include "../../LightAnimLibrary.h"

CUICustomEdit::CUICustomEdit()
{
	m_max_symb_count = u32(-1);

	m_bShift = false;
	m_bAlt = false;
	m_bInputFocus = false;
   
	m_iKeyPressAndHold = 0;
	m_bHoldWaitMode = false;

	m_lines.SetVTextAlignment(valCenter);
	m_lines.SetColoringMode(false);
	m_lines.SetCutWordsMode(true);
	m_lines.SetUseNewLineMode(false);
	SetText("");
	m_textPos.set(3,0);
	m_bNumbersOnly = false;
	m_bFloatNumbers = false;
	m_bFocusByDbClick = false;

	m_textColor[0]=color_argb(255,235,219,185);
	m_textColor[1]=color_argb(255,100,100,100);
}

CUICustomEdit::~CUICustomEdit()
{
}

void CUICustomEdit::SetTextColor(u32 color){
	m_textColor[0] = color;
}

void CUICustomEdit::SetTextColorD(u32 color){
	m_textColor[1] = color;
}

void CUICustomEdit::Init(float x, float y, float width, float height){
	CUIWindow::Init(x,y,width,height);
	m_lines.SetWndSize(m_wndSize);
}

void CUICustomEdit::SetLightAnim(LPCSTR lanim)
{
	if(lanim&&xr_strlen(lanim))
		m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim	= NULL;
}

void CUICustomEdit::SetPasswordMode(bool mode){
	m_lines.SetPasswordMode(mode);
}

void CUICustomEdit::OnFocusLost(){
	CUIWindow::OnFocusLost();
}

void CUICustomEdit::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
//	if(pWnd == GetParent())
//	{
		//���-�� ������ �������� ����������
		if(msg == WINDOW_KEYBOARD_CAPTURE_LOST)
		{
			m_bInputFocus = false;
			m_iKeyPressAndHold = 0;
		}
//	}
}

bool CUICustomEdit::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if (m_bFocusByDbClick)
	{
		if(mouse_action == WINDOW_LBUTTON_DB_CLICK && !m_bInputFocus)
		{
			GetParent()->SetKeyboardCapture(this, true);
			m_bInputFocus = true;
			m_iKeyPressAndHold = 0;

			m_lines.MoveCursorToEnd();
		}
	}

	if(mouse_action == WINDOW_LBUTTON_DOWN && !m_bInputFocus)
	{
		GetParent()->SetKeyboardCapture(this, true);
		m_bInputFocus = true;
		m_iKeyPressAndHold = 0;

		m_lines.MoveCursorToEnd();
	}
	return false;
}


bool CUICustomEdit::OnKeyboard(int dik, EUIMessages keyboard_action)
{
	if (!m_bInputFocus)
		return false;

	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		m_iKeyPressAndHold = dik;
		m_bHoldWaitMode = true;

		if (KeyPressed(dik))
			return true;
	}
	else if (keyboard_action == WINDOW_KEY_RELEASED)
	{
		if (m_iKeyPressAndHold == dik)
		{
			m_iKeyPressAndHold = 0;
			m_bHoldWaitMode = false;
		}
		if (KeyReleased(dik))
			return true;
	}
	return false;
}

bool CUICustomEdit::KeyPressed(int dik)
{
	bool bChanged = false;

	switch(dik)
	{
	case DIK_LEFT:
	case DIKEYBOARD_LEFT:
		m_lines.DecCursorPos();		
		break;
	case DIK_RIGHT:
	case DIKEYBOARD_RIGHT:
		m_lines.IncCursorPos();		
		break;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		m_bShift = true;
		if (m_bAlt)
			PostMessage(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 2, 0); //����������� ����
		break;
	case DIK_ESCAPE:
		if (xr_strlen(GetText()) != 0)
		{
			SetText("");
			bChanged = true;
		}
		else
		{
			GetParent()->SetKeyboardCapture(this, false);
			m_bInputFocus = false;
			m_iKeyPressAndHold = 0;
		};
		break;
	case DIK_RETURN:
	case DIK_NUMPADENTER:
		GetParent()->SetKeyboardCapture(this, false);
		m_bInputFocus = false;
		m_iKeyPressAndHold = 0;
		GetMessageTarget()->SendMessage(this,EDIT_TEXT_COMMIT,NULL);
		break;
	case DIK_BACKSPACE:
		m_lines.DelLeftChar();
		bChanged = true;
		break;
	case DIK_DELETE:
	case DIKEYBOARD_DELETE:
		m_lines.DelChar();
		bChanged = true;
		break;
	case DIK_LALT:
	case DIK_RALT:
		m_bAlt = true;
		break;
	default:
		char out_me = 0;
		auto layout = GetKeyboardLayout(0);
		if (m_bShift)
		{
			if ((int)layout == 67699721) //�������� ���������� ���������
				switch (dik)
				{
				case DIK_LBRACKET:   out_me = '{'; break;
				case DIK_RBRACKET:   out_me = '}'; break;
				case DIK_SEMICOLON:  out_me = ':'; break;
				case DIK_APOSTROPHE: out_me = '"'; break;
				case DIK_COMMA:      out_me = '<'; break;
				case DIK_PERIOD:     out_me = '>'; break;
				}
			if (!out_me)
				switch (dik)
				{
				case DIK_BACKSLASH: out_me = '|'; break;
				case DIK_SLASH:     out_me = '?'; break;
				case DIK_MINUS:     out_me = '_'; break;
				case DIK_EQUALS:    out_me = '+'; break;
				}
		}
		if (!out_me)
		{
			static byte State[256];
			if (!GetKeyboardState(State)) { Msg("!![CUICustomEdit::KeyPressed] GetKeyboardState() == 0 !!!"); return true; };
			auto vk = MapVirtualKeyEx(dik, 1, layout);
			char c[4];
			if (ToAsciiEx(vk, dik, State, (unsigned short *)c, 0, layout) == 1)
			{
				out_me = c[0];
				if (m_bShift)
					switch (out_me)
					{
					case '1': out_me = '!';	break;
					case '2': out_me = '@';	break;
					case '3': out_me = '#';	break;
					case '4': out_me = '$';	break;
					case '5': out_me = '%';	break;
					case '6': out_me = '^';	break;
					case '7': out_me = '&';	break;
					case '8': out_me = '*';	break;
					case '9': out_me = '(';	break;
					case '0': out_me = ')';	break;
					default:
						if ((out_me >= '�' && out_me <= '�') || out_me == '�' || (out_me >= 'a' && out_me <= 'z')) //������ �������� ������ ������� ��������
							//������� �������� � ���������� ANSI, � ������������������ �������� � ���� ��������� ������ ������� (����� �������� '�' - '�')
							if (out_me == '�')
								out_me = '�';
							else
								out_me -= 32;
					}
				else if ((out_me >= '�' && out_me <= '�') || out_me == '�' || (out_me >= 'A' && out_me <= 'Z')) //�������, ���� �� ���� ������� ��� ������� �����
					if (out_me == '�')
						out_me = '�';
					else
						out_me += 32;
			}
		}
		if (out_me)
			if (!m_bNumbersOnly || ((!m_bShift && out_me >= '0' && out_me <= '9') || (m_bFloatNumbers && out_me == '.' && !strstr(m_lines.GetText(), "."))))
			{
				AddChar(out_me);
				bChanged = true;
			}
	}

	if(bChanged)
		GetMessageTarget()->SendMessage(this,EDIT_TEXT_CHANGED,NULL);

	return true;
}


bool CUICustomEdit::KeyReleased(int dik)
{
	switch (dik)
	{
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		m_bShift = false;
		break;
	case DIK_LALT:
	case DIK_RALT:
		m_bAlt = false;
		break;
	}

	return true;
}


void CUICustomEdit::AddChar(char c)
{
	if(xr_strlen(m_lines.GetText()) >= m_max_symb_count)
		return;

	float text_length	= m_lines.GetFont()->SizeOf_(m_lines.GetText());
	UI()->ClientToScreenScaledWidth		(text_length);

	if (!m_lines.GetTextComplexMode() && (text_length > GetWidth() - 1))
		return;

	m_lines.AddCharAtCursor(c);
	m_lines.ParseText();
	if (m_lines.GetTextComplexMode())
		if (m_lines.GetVisibleHeight() > GetHeight())
			m_lines.DelLeftChar();
}


//����� ��� ������������� ���������
//������� ��� ������������ ������
#define HOLD_WAIT_TIME 400
#define HOLD_REPEAT_TIME 100

void CUICustomEdit::Update()
{
	if (m_bInputFocus)
	{
		static u32 last_time;

		u32 cur_time = Device.TimerAsync();

		if (m_iKeyPressAndHold)
		{
			if (m_bHoldWaitMode)
			{
				if (cur_time - last_time>HOLD_WAIT_TIME)
				{
					m_bHoldWaitMode = false;
					last_time = cur_time;
				}
			}
			else
			{
				if (cur_time - last_time>HOLD_REPEAT_TIME)
				{
					last_time = cur_time;
					KeyPressed(m_iKeyPressAndHold);
				}
			}
		}
		else
			last_time = cur_time;
	}

	m_lines.SetTextColor(m_textColor[IsEnabled() ? 0 : 1]);
	CUIWindow::Update();
}

void CUICustomEdit::Draw()
{
	CUIWindow::Draw			();
	Fvector2				pos;
	GetAbsolutePos			(pos);
	m_lines.Draw			(pos.x + m_textPos.x, pos.y + m_textPos.y);
	
	if(m_bInputFocus)
	{ //draw cursor here
		Fvector2							outXY;
		
		outXY.x								= 0.0f;
		float _h				= m_lines.m_pFont->CurrentHeight_();
		UI()->ClientToScreenScaledHeight(_h);
		outXY.y								= pos.y + (GetWndSize().y - _h)/2.0f;

		float								_w_tmp;
		int i								= m_lines.m_iCursorPos;
		string256							buff;
		strncpy								(buff,m_lines.m_text.c_str(),i);
		buff[i]								= 0;
		_w_tmp								= m_lines.m_pFont->SizeOf_(buff);
		UI()->ClientToScreenScaledWidth		(_w_tmp);
		outXY.x								= pos.x + _w_tmp;
		
		_w_tmp								= m_lines.m_pFont->SizeOf_("-");
		UI()->ClientToScreenScaledWidth		(_w_tmp);
		UI()->ClientToScreenScaled			(outXY);

		m_lines.m_pFont->Out				(outXY.x, outXY.y, "_");
	}
}

void CUICustomEdit::SetText(LPCSTR str)
{
	CUILinesOwner::SetText(str);
}

const char* CUICustomEdit::GetText(){
	return CUILinesOwner::GetText();
}

void CUICustomEdit::Enable(bool status){
	CUIWindow::Enable(status);
	if (!status)
		SendMessage(this,WINDOW_KEYBOARD_CAPTURE_LOST);
}

void CUICustomEdit::SetNumbersOnly(bool status){
	m_bNumbersOnly = status;
}

void CUICustomEdit::SetFloatNumbers(bool status){
	m_bFloatNumbers = status;
}
