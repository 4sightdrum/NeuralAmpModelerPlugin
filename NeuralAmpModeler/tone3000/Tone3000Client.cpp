#include "Tone3000Client.h"
#include <cstdio>
#include <sstream>
#include <stdexcept>

// Helpers (same scheme as your working test)
static void splitHttp(const std::string& in, std::string& body, std::string& code) {
  body.clear(); code.clear();
  const std::string M="--HTTP-CODE--";
  size_t m=in.rfind(M);
  if(m!=std::string::npos){
    body=in.substr(0,m);
    size_t s=m+M.size();
    while(s<in.size() && (in[s]==':'||in[s]==' '||in[s]=='\t')) ++s;
    size_t e=s;
    while(e<in.size() && in[e] != '\n' && in[e] != '\r') ++e;
    code=in.substr(s,e-s);
    return;
  }
  std::string s=in;
  while(!s.empty() && (s.back()=='\n'||s.back()=='\r')) s.pop_back();
  size_t nl=s.find_last_of("\r\n");
  if(nl==std::string::npos){ body=s; return; }
  body=s.substr(0,nl);
  code=s.substr(nl+1);
}

static std::string jget(const std::string& json, const std::string& key) {
  std::string pat = "\"" + key + "\":\"";
  size_t p=json.find(pat);
  if(p==std::string::npos) return {};
  p += pat.size();
  size_t q = json.find('"', p);
  if(q==std::string::npos) return {};
  return json.substr(p, q-p);
}

std::pair<std::string,int> Tone3000Client::Curl(const std::string& cmd) {
  std::string out; char buf[4096];
  FILE* pipe = popen((cmd + " -w \"\n--HTTP-CODE--%{http_code}\n\" 2>/dev/null").c_str(), "r");
  if(!pipe) throw std::runtime_error("curl failed");
  while(fgets(buf, sizeof(buf), pipe)) out += buf;
  pclose(pipe);
  std::string body, code;
  splitHttp(out, body, code);
  int http = code.empty() ? -1 : std::stoi(code);
  return { body, http };
}

T3KSession Tone3000Client::CreateSession(const std::string& apiKey) {
  std::ostringstream cmd;
  cmd << "curl -s -X POST \"https://www.tone3000.com/api/v1/auth/session\" "
      << "-H \"Content-Type: application/json\" "
      << "-d '{\"api_key\":\"" << apiKey << "\"}'";
  auto [body,code] = Curl(cmd.str());
  if(code != 200) throw std::runtime_error("session http " + std::to_string(code) + ": " + body);
  return { jget(body,"access_token"), jget(body, "refresh_token") };
}

T3KUser Tone3000Client::GetUser(const std::string& access) {
  std::ostringstream cmd;
  cmd << "curl -s -X GET \"https://www.tone3000.com/api/v1/user\" "
      << "-H \"Authorization: Bearer " << access << "\" "
      << "-H \"Content-Type: application/json\"";
  auto [body,code] = Curl(cmd.str());
  if(code != 200) throw std::runtime_error("user http " + std::to_string(code) + ": " + body);
  return { jget(body,"id"), jget(body,"username"), jget(body,"url") };
}

std::vector<T3KTone> Tone3000Client::GetCreatedTones(const std::string& access, int page, int pageSize) {
  std::ostringstream cmd;
  cmd << "curl -s -X GET \"https://www.tone3000.com/api/v1/tones/created?page=" << page << "&page_size=" << pageSize << "\" "
      << "-H \"Authorization: Bearer " << access << "\" "
      << "-H \"Content-Type: application/json\"";
  auto [body,code] = Curl(cmd.str());
  if(code != 200) throw std::runtime_error("tones http " + std::to_string(code) + ": " + body);
  std::vector<T3KTone> v;
  size_t pos = 0;
  while((pos = body.find("\"title\":\"", pos)) != std::string::npos) {
    pos += 9;
    size_t end = body.find('"', pos);
    T3KTone t;
    t.title = body.substr(pos, end - pos);
    v.push_back(t);
    pos = end + 1;
  }
  return v;
}

T3KSession Tone3000Client::Refresh(const std::string& access, const std::string& refresh) {
  std::ostringstream cmd;
  cmd << "curl -s -X POST \"https://www.tone3000.com/api/v1/auth/session/refresh\" "
      << "-H \"Content-Type: application/json\" "
      << "-d '{\"refresh_token\":\"" << refresh << "\",\"access_token\":\"" << access << "\"}'";
  auto [body,code] = Curl(cmd.str());
  if(code != 200) throw std::runtime_error("refresh http " + std::to_string(code) + ": " + body);
  return { jget(body,"access_token"), jget(body,"refresh_token") };
}
