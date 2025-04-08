# Installation Instructions for using MSYS2 UCRT64 Enviornment

For this project, I decided to install and use MSYS2 for the enviornment since I can run Python and C together on a Windows PC. This made it simple to integrate into VS Code or on a separate terminal. You will need to also install Python, CMake, Ninja, and GCC into the MSYS2 environment.

## Installing MSYS2

For easy install, follow the directions on MSYS2's website at msys2.org. Make sure you are on a Windows 10 or newer

## Installing tools and languages

Once you have MSYS2 installed and have confirmed that it is in your PATH variables, start up the enviornment. In VS Code, I just choose the **MSYS2 UCRT** option to make it simple and start a new terminal. 

Follow these commands:

```
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-ninja
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-python
```

You can run all 4 at the same time. I like to restart the terminal and then check the versions of each by running:

```
cmake --version
ninja --version
gcc --version
python --version
```

If they are not recognized, check the path in your terminal by running:

```
echo $PATH
```

You may need to add them to the PATH variable. 

### Tips

- Always update pacman by running ```pacman -Syu```
- Make sure the install files have ucrt in the path
- Restarting the terminal is a good standard "Have you turned it off and on again" fix