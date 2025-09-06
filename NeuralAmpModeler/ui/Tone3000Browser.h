#pragma once
#include "IControl.h"
#include <vector>
#include <string>
#include "tone3000/Tone3000Client.h"

class Tone3000Browser : public IControl {
public:
  Tone3000Browser(const IRECT& r);
  void Draw(IGraphics& g) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnTextEntryCompletion(const char* str, int) override;

private:
  enum class State { Idle, SigningIn, Ready, Loading, Error };
  State mState = State::Idle;
  std::string mStatus, mAPIKey, mAccess, mRefresh;
  std::vector<T3KTone> mTones;
  int mSelected = -1;

  void SetStatus(const std::string& s);
  void SignIn();
  void FetchTones();
};
