This package contains static SQLite libraries and header files
for the x64 platform and Debug/Release configurations  built with
Visual C++ 2022, against Debug/Release MT/DLL MSVC CRT.

The SQLite static library appropriate for the platform and
configuration selected in a Visual Studio solution is explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. This library may
be moved into any solution folder after the installation.

Note that the SQLite library path in this package will be selected
as Debug or Release based on whether the active configuration
is designated as a development or as a release configuration in
the underlying .vcxproj file.

Specifically, the initial project configurations have a property
called UseDebugLibraries in the underlying .vcxproj file, which
reflects whether the configuration is intended for building release
or development artifacts. Additional configurations copied from
these initial ones inherit this property. Manually created
configurations should have this property defined in the .vcxproj
file.

Do not install this package if your projects use configurations
without the UseDebugLibraries property.

See project repository for more information.

https://github.com/StoneStepsInc/sqlite-nuget

