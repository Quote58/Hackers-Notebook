# The Hackers Notebook

<img width="962" alt="Screenshot 2023-01-22 at 7 05 39 PM" src="https://user-images.githubusercontent.com/15618772/213947723-9551b82a-48f0-43ae-ad1e-6df63ca9ed94.png">
The Hackers Notebook is a program designed to solve two problems.

The **first** is a lack of hex editors available which are:
- Cross platform
- Intuitive
- Functional
- Fun to use

The **second**, is a lack of direction and consolodation of tools for basic rom hacking and documentation. More specifically, much of the exploration phase of rom hacking is tedious and error prone, and often relies on searching through data in a hex editor.

**To address the first, The Hackers Notebook:**
- Is built in WxWidgets, a cross platform GUI which is well known, looks appropriate on every platform, and uses C++
- Follows basic design principles for an intuitive program
- Has many functions specifically for rom hacking, but also provides the functionality of a hex editor and documentation builder regardless of use case
- Has the goal of making basic rom hacking fun by removing some of the barriers to entry and some of the more confusing aspects of rom hacking

**To address the Second:**
The hackers notebook seeks to consolodate many of the programs needed for basic rom hacking such as a hex editor, gfx viewer, palette editor, patch making tool, and basic documentation editor. This consolodation leads to a synergy between these tools as well. The hex editor can add document entries and create patches based on recent changes, while the documentation editor naturally creates bookmarks for the hex editor. The goal is that all three elements of the program (documentation, rom editing, patches) work in tandem to make the process of basic rom hacking (translation, gfx editing, palette editing, rom searching and documenting, simple patches, etc.) a seamless process that works the same way on any desktop operating system (in fact, it is being developed on MacOS, an operating system which is tradionally not well supported in rom hacking tools).

Let's take a look at some of the features!
## Documentation
Documentation is a large part of the Hackers Notebook. A document entry consists of a title, category, optional longer description, and an address. This allows the user to quickly see what the address is, and if a longer description is needed, they can click the '?' button next to the entry.
<img width="962" alt="Document view" src="https://user-images.githubusercontent.com/15618772/213947800-c222f543-5824-4645-9057-e42e9dc48dc2.png">
<img width="962" alt="Optional description" src="https://user-images.githubusercontent.com/15618772/213948367-38022c43-0945-442c-80a9-a260b1eeb9b9.png">

The address of a documentation entry also doubles as a bookmark for the hex editor. Double clicking the address will take you to the specific offset within the hex editor. Documentation will also be able to export in different formats, be searched and sorted, and more. A documentation entry can also optionally contain the byte data within a specified range.

## Patches
Creating patches for small changes, like so-called 'hex edits' has always been a little bit error prone. If you wanted to test out a patch or hex edit and you forgot to make a back up, you would have to manually reverse those changes to the rom. In The Hackers Notebook, patches are made to be applied and reversed with the click of a button. They are displayed with a checkbox to say whether the patch is applied, not applied, or does not match the current bytes in those locations. This allows the user to easily make and test patches without having to worry about reversing or remember what changes were made. And just like document entries, patch entries can also contain an optional longer form description.
<img width="918" alt="Patches view" src="https://user-images.githubusercontent.com/15618772/213948505-401a6c76-d7c7-4d35-bb86-88288372aba1.png">

## Rom Editor
The rom editor is at its core a hex editor (with support for tables and other features helpful for rom hacking), but it's got a trick up its sleave. It can also render the rom in other ways. You don't only have to look at the rom as byte data, you can search through the rom while viewing it as characters (with support for multi-byte character tables), colour data, or even graphics data. This allows the user to quickly identify different parts of the rom, and ultimately, will allow for editing in each type of view mode. It currently supports editing in byte, character, and palette modes.
<img width="962" alt="Byte Data" src="https://user-images.githubusercontent.com/15618772/213948717-e5d1b972-1121-49ee-a26f-20a675e877d6.png">
<img width="962" alt="String data" src="https://user-images.githubusercontent.com/15618772/213948913-3f12b176-675e-44fa-974c-4960ad0fadf5.png">
<img width="962" alt="SNES palette data" src="https://user-images.githubusercontent.com/15618772/213948949-9e7e6bd8-f8e4-4295-ba78-9278e0e02289.png">
<img width="962" alt="SNES Samus" src="https://user-images.githubusercontent.com/15618772/213949130-66175754-aee6-4a51-8a9f-a1d4971913af.png">
<img width="962" alt="NES Mario" src="https://user-images.githubusercontent.com/15618772/213949007-3b026c30-27a8-4062-abb5-dd9d0627db70.png">

You can even edit colours with the native system colour picker, and it will automatically convert up and down from 24bit colour to whichever bitdepth you have currently selected!

![Dec-09-2022_21-40-53](https://user-images.githubusercontent.com/15618772/213949256-8e48c664-2ac2-405e-bb0c-150c897ac3ae.gif)

