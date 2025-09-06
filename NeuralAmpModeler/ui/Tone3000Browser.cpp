#include "Tone3000Browser.h"
#include "ITextEntryControl.h"
#include <thread>

Tone3000Browser::Tone3000Browser(const IRECT& r) : IControl(r)
{
  SetWantsKeyboardFocus(true);
}

void Tone3000Browser::SetStatus(const std::string& s)
{
  mStatus = s;
  SetDirty(false);
}

void Tone3000Browser::Draw(IGraphics& g)
{
  g.FillRect(COLOR_BLACK, mRECT);
  auto header = mRECT.FracRectVertical(0.18f, true);
  g.DrawText(IText(16, COLOR_WHITE), "Tone3000 Browser", header.GetPadded(-6.f));

  auto row = header.GetFromBottom(header.H()*0.55f).GetPadded(-6.f);
  IRECT keyBox = row.SubRectHorizontal(0.6f, true);
  IRECT signIn = row.SubRectHorizontal(0.2f, true);
  IRECT refresh = row;

  g.FillRect(COLOR_DARK_GREY, keyBox);
  g.DrawText(IText(12, COLOR_WHITE), mAPIKey.empty() ? "Click to enter API key" : mAPIKey.c_str(), keyBox);

  g.FillRect(COLOR_BLUE, signIn);
  g.DrawText(IText(12, COLOR_WHITE), "Sign In", signIn);

  g.FillRect(COLOR_32, refresh);
  g.DrawText(IText(12, COLOR_WHITE), "Refresh", refresh);

  auto list = mRECT.GetPadded(8.f).GetFromBottom(mRECT.H()*0.75f);
  g.FillRect(COLOR_25, list);
  const float rowH = 18.f;
  int maxRows = int(list.H()/rowH);
  for(int i=0; i<maxRows && i<(int)mTones.size(); ++i)
  {
    IRECT r = IRECT(list.L, list.T + i*rowH, list.R, list.T + (i+1)*rowH);
    g.FillRect((i==mSelected)? COLOR_MID_GREY : COLOR_TRANSPARENT, r);
    g.DrawText(IText(12, COLOR_WHITE), mTones[i].title.c_str(), r.GetPadded(-4.f));
  }
  g.DrawText(IText(11, COLOR_LIGHT_GREY), mStatus.c_str(), mRECT.GetFromBottom(18.f).GetPadded(-4.f));
}

void Tone3000Browser::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  auto header = mRECT.FracRectVertical(0.18f, true);
  auto row = header.GetFromBottom(header.H()*0.55f).GetPadded(-6.f);
  IRECT keyBox = row.SubRectHorizontal(0.6f, true);
  IRECT signIn = row.SubRectHorizontal(0.2f, true);
  IRECT refresh = row;
  if(keyBox.Contains(x,y))
  {
    GetUI()->CreateTextEntry(*this, IText(12), keyBox, "");
    return;
  }
  if(signIn.Contains(x,y)) { SignIn(); return; }
  if(refresh.Contains(x,y)) { FetchTones(); return; }
  auto list = mRECT.GetPadded(8.f).GetFromBottom(mRECT.H()*0.75f);
  const float rowH = 18.f;
  int idx = int((y - list.T)/rowH);
  if(list.Contains(x,y) && idx>=0 && idx<(int)mTones.size())
  {
    mSelected = idx;
    SetDirty(false);
  }
}

void Tone3000Browser::OnTextEntryCompletion(const char* str, int)
{
  if(str) mAPIKey = str;
}

void Tone3000Browser::SignIn()
{
  if(mAPIKey.empty())
  {
    SetStatus("Enter API key first");
    return;
  }
  mState = State::SigningIn;
  SetStatus("Signing in...");
  std::thread([this]{
    try
    {
      auto s = Tone3000Client::CreateSession(mAPIKey);
      mAccess = s.access;
      mRefresh = s.refresh;
      GetUI()->OnUIThread([this]{
        mState = State::Ready;
        SetStatus("Signed in");
        FetchTones();
      });
    }
    catch(const std::exception& e)
    {
      GetUI()->OnUIThread([this, e]{ mState = State::Error; SetStatus(e.what()); });
    }
  }).detach();
}

void Tone3000Browser::FetchTones()
{
  if(mAccess.empty())
  {
    SetStatus("Sign in first");
    return;
  }
  mState = State::Loading;
  SetStatus("Loading tones...");
  std::thread([this]{
    try
    {
      auto v = Tone3000Client::GetCreatedTones(mAccess, 1, 50);
      GetUI()->OnUIThread([this, v]{
        mTones = v;
        SetStatus(std::to_string((int)mTones.size()) + " tones");
        SetDirty(false);
      });
    }
    catch(const std::exception& e)
    {
      try
      {
        auto s = Tone3000Client::Refresh(mAccess, mRefresh);
        mAccess = s.access;
        mRefresh = s.refresh;
        auto v = Tone3000Client::GetCreatedTones(mAccess, 1, 50);
        GetUI()->OnUIThread([this, v]{
          mTones = v;
          SetStatus(std::to_string((int)mTones.size()) + " tones");
          SetDirty(false);
        });
      }
      catch(const std::exception& e2)
      {
        GetUI()->OnUIThread([this, e2]{ mState = State::Error; SetStatus(e2.what()); });
      }
    }
  }).detach();
}
