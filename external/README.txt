This folder is generally intended to put all the crap that needs to be found from other libraries in a single place so that the configuation of the visual studio project is a little bit easier without having a ton of include paths and library paths added to the project.

In addition, there's a group of DLL's that are needed by the app and a stupid install script to copy all those DLL's to C:\Windows\System

You could slap those DLL's in the same folder as the application if you wanted to do a localized installation and/or if the DLL's might conflict with something else installed on the computer, but I prefer to keep as many files out of the directory with the source code in it as possible while I am developing personally.
