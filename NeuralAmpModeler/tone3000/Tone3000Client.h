#pragma once
#include <string>
#include <vector>
#include <utility>

struct T3KSession { std::string access, refresh; };
struct T3KUser    { std::string id, username, url; };
struct T3KTone    { std::string id, title; };

class Tone3000Client {
public:
  static std::pair<std::string,int> Curl(const std::string& cmd);
  static T3KSession CreateSession(const std::string& apiKey);
  static T3KUser    GetUser(const std::string& access);
  static std::vector<T3KTone> GetCreatedTones(const std::string& access, int page=1, int pageSize=50);
  static T3KSession Refresh(const std::string& access, const std::string& refresh);
};
