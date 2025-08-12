#include "ToneAPI.h"

static juce::URL::InputStreamOptions opts()
{
    return juce::URL::InputStreamOptions()
            .withConnectionTimeoutMs(8000)
            .withNumRedirectsToFollow(5);
}

ToneAPI::ToneAPI(juce::URL base, juce::String apiKey)
: baseUrl(base), key(apiKey)
{
}

juce::URL ToneAPI::withAuth(juce::URL u) const
{
    if (key.isNotEmpty())
        u = u.withParameter("apiKey", key);
    return u;
}

juce::Result ToneAPI::getTones(juce::Array<ToneItem>& out)
{
    auto url = withAuth(baseUrl.withNewSubPath("api/v1/tones"));
    std::unique_ptr<juce::InputStream> in(url.createInputStream(opts()));
    if (!in)
        return juce::Result::fail("Network error");

    auto json = juce::JSON::parse(in->readEntireStreamAsString());
    if (json.isVoid() || !json.isArray())
        return juce::Result::fail("Bad JSON");
    for (auto* v : *json.getArray())
    {
        auto* o = v->getDynamicObject();
        if (!o) continue;
        ToneItem ti;
        ti.id       = o->getProperty("id").toString();
        ti.name     = o->getProperty("name").toString();
        ti.author   = o->getProperty("author").toString();
        ti.category = o->getProperty("category").toString();
        ti.downloadUrl = juce::URL(o->getProperty("download_url").toString());
        if (ti.downloadUrl.isWellFormed())
            out.add(ti);
    }
    return juce::Result::ok();
}

juce::Result ToneAPI::downloadTone(const ToneItem& t, juce::File& outFile)
{
    auto url = withAuth(t.downloadUrl);
    std::unique_ptr<juce::InputStream> in(url.createInputStream(opts()));
    if (!in)
        return juce::Result::fail("Network error");

    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("Tone3000NAM").getChildFile("Models");
    dir.createDirectory();
    outFile = dir.getChildFile(t.name + ".nam");

    juce::MemoryBlock buffer;
    in->readIntoMemoryBlock(buffer);
    if (buffer.getSize() == 0)
        return juce::Result::fail("Empty model data");
    if (!outFile.replaceWithData(buffer.getData(), buffer.getSize()))
        return juce::Result::fail("Failed to write model");
    return juce::Result::ok();
}
