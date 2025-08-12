#include "ToneBrowserPanel.h"
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

ToneBrowserPanel::ToneBrowserPanel(NeuralAmpModelerAudioProcessor& p, ToneAPI api_)
: processor(p), api(api_)
{
    addAndMakeVisible(search);
    addAndMakeVisible(refreshBtn);
    addAndMakeVisible(list);

    search.setTextToShowWhenEmpty("Search tones…", juce::Colours::grey);
    search.addListener(this);

    refreshBtn.setButtonText("Refresh");
    refreshBtn.addListener(this);

    list.setModel(this);
    list.setRowHeight(24);

    refresh();
}

void ToneBrowserPanel::resized()
{
    auto r = getLocalBounds().reduced(8);
    auto top = r.removeFromTop(28);
    search.setBounds(top.removeFromLeft(r.getWidth() - 100));
    refreshBtn.setBounds(top.removeFromLeft(100).reduced(4));
    list.setBounds(r);
}

void ToneBrowserPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.8f));
}

int ToneBrowserPanel::getNumRows()
{
    return (int) filtered.size();
}

void ToneBrowserPanel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= filtered.size()) return;
    auto& t = *filtered.getUnchecked(rowNumber);
    if (rowIsSelected) g.fillAll(juce::Colours::darkblue.withAlpha(0.4f));
    g.setColour(juce::Colours::whitesmoke);
    g.drawText(t.name, 8, 0, width/2, height, juce::Justification::centredLeft, true);
    g.setColour(juce::Colours::grey);
    g.drawText(t.category + "  \u2022  " + t.author, width/2, 0, width/2 - 8, height, juce::Justification::centredRight, true);
}

void ToneBrowserPanel::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < filtered.size())
        loadTone(*filtered.getUnchecked(row));
}

void ToneBrowserPanel::buttonClicked(juce::Button* b)
{
    if (b == &refreshBtn)
        refresh();
}

void ToneBrowserPanel::textEditorTextChanged(juce::TextEditor&)
{
    applyFilter();
}

void ToneBrowserPanel::refresh()
{
    juce::Array<ToneItem> items;
    if (auto res = api.getTones(items); res.wasOk())
    {
        all.clear(true);
        for (auto& item : items)
            all.add(new ToneItem(item));
        applyFilter();
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                "Tone3000", res.getErrorMessage());
    }
}

void ToneBrowserPanel::applyFilter()
{
    auto q = search.getText().trim().toLowerCase();
    filtered.clear(true);
    for (auto* t : all)
    {
        if (q.isEmpty() || t->name.toLowerCase().contains(q) || t->author.toLowerCase().contains(q) || t->category.toLowerCase().contains(q))
            filtered.add(new ToneItem(*t));
    }
    list.updateContent();
    list.repaint();
}

void ToneBrowserPanel::loadTone(const ToneItem& item)
{
    juce::File f;
    if (auto res = api.downloadTone(item, f); res.wasOk())
    {
        // TODO: call NAM's model-loading method. Replace the following with real call.
        // Example: processor.loadModelFromFile(f);
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                               "Tone loaded", "Loaded: " + f.getFileName());
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "Download failed", res.getErrorMessage());
    }
}
