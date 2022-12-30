# Marketeer
A tool for broadcasting Touhou games without needing to send a video feed - by only sending inputs and some game state and play that back on the other end

> I will indefinitely cease all future development on thprac/Marketeer, you are free to continue working on it, granting that you follow the license's terms.
> Due to my insufficient skill, the code is glutted with obscure writings and terrible logic, turning the whole thing into a complete mess. I'm sorry if that disturbs you.
- Ack

This project was originally integrated into thprac and developed as part of thprac, but I began separating the 2. There are still thprac leftovers left but aside from that the code quality is still very bad :hahaa:

# How to build:
Both thprac and marketeer_server include Visual Studio solutuions that can be built with Visual Studio 2022, but make sure that Windows XP support for the Visual Studio 2017 (v141) tools is installed.
marketeer_server includes a Visual Studio solution for building on Windows but the code can be built on Linux (assuming wolfssl and nng are installed through your package manager) by just running `make`
