#pragma once
#include <JuceHeader.h>
#include "ToneAPI.h"

class NeuralAmpModelerAudioProcessor;

class ToneBrowserPanel : public juce::Component,
                         private juce::ListBoxModel,
                         private juce::Button::Listener,
                         private juce::TextEditor::Listener
{
public:
    ToneBrowserPanel(NeuralAmpModelerAudioProcessor& processor, ToneAPI api);

    void resized() override;
    void paint(juce::Graphics& g) override;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

    // Button::Listener override
    void buttonClicked(juce::Button*) override;

    // TextEditor listener
    void textEditorTextChanged(juce::TextEditor&) override;

private:
    void refresh();
    void applyFilter();
    void loadTone(const ToneItem& t);

    NeuralAmpModelerAudioProcessor& processor;
    ToneAPI api;

    juce::OwnedArray<ToneItem> all;
    juce::OwnedArray<ToneItem> filtered;

    juce::TextEditor search;
    juce::TextButton refreshBtn;
    juce::ListBox list;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneBrowserPanel)
};
