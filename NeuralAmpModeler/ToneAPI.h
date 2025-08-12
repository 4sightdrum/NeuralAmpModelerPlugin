#pragma once
#include <JuceHeader.h>

struct ToneItem
{
    juce::String id, name, author, category;
    juce::URL downloadUrl;
};

class ToneAPI
{
public:
    ToneAPI(juce::URL base, juce::String apiKey = {});
    juce::Result getTones(juce::Array<ToneItem>& out);
    juce::Result downloadTone(const ToneItem& t, juce::File& outFile);

private:
    juce::URL withAuth(juce::URL u) const;
    juce::URL baseUrl;
    juce::String key;
};
