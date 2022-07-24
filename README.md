## SQLite Nuget Package

This project builds a SQLite Nuget package with static SQLite
libraries and header files  for the `x64` platform and
`Debug`/`Release` configurations.

Visit SQLite project for SQLite library documentation:

https://www.sqlite.org/

## Package Configuration

This package contains only static libraries for platforms and
configurations listed above.

The SQLite static library appropriate for the platform and
configuration selected in a Visual Studio solution is explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. This library may
be moved into any solution folder after the installation.

Note that the SQLite library path in this package is valid only
for build configurations named `Debug` and `Release` and will not
work for any other configuration names. Do not install this package
for projects with configurations other than `Debug` and `Release`.

See `StoneSteps.SQLite.Static.props` and `StoneSteps.SQLite.Static.targets`
for specific package configuration details and file locations.

## Package Version

### Package Revision

Nuget packages lack package revision and in order to repackage
the same upstream software version, such as SQLite v3.38.2, the
4th component of the Nuget version is used to track the Nuget
package revision.

Nuget package revision is injected outside of the Nuget package
configuration, during the package build process, and is not present
in the package specification file.

Specifically, `nuget.exe` is invoked with `-Version=3.38.2.123` to
build a package with the revision `123`.

### Version Locations

SQLite version is located in a few places in this repository and
needs to be changed in all of them for a new version of SQLite.

  * nuget/StoneSteps.SQLite.Static.nuspec (`version`)
  * devops/make-package.bat (`PKG_VER`, `PKG_VER_ABBR`)
  * .github/workflows/nuget-sqlite-3.38.2.yml (`name`, `PKG_VER`,
    `PKG_REV`, `SQLITE_FNAME`)

In the GitHub workflow YAML, `PKG_REV` must be reset to `1` (one)
every time SQLite version is changed. The workflow file must
be renamed with the new version in the name. This is necessary
because GitHub maintains build numbers per workflow file name.

For local builds package revision is supplied on the command line
and should be specified as `1` (one) for a new version of SQLite.

### GitHub Build Number

Build number within the GitHub workflow YAML is maintained in an
unconventional way because of the lack of build maturity management
between GitHub and Nuget.

For example, using a build management system, such as Artifactory,
every build would generate a Nuget package with the same version
and package revision for the upcoming release and build numbers
would be tracked within the build management system. A build that
was successfully tested would be promoted to the production Nuget
repository without generating a new build.

Without a build management system, the GitHub workflow in this
repository uses the pre-release version as a surrogate build
number for builds that do not publish packages to nuget.org, so
these builds can be downloaded and tested before the final build
is made and published to [nuget.org][]. This approach is not
recommended for robust production environments because even
though the final published package is built from the exact same
source, the build process may still potentially introduce some
unknowns into the final package (e.g. build VM was updated).

## Building Package Locally

You can build a Nuget package locally with `make-package.bat`
located in `devops`. This script expects VS2019 Community Edition
installed in the default location. If you have other edition of
Visual Studio, edit the file to use the correct path to the
`vcvarsall.bat` file.

Run `make-package.bat` from the repository root directory with
a package revision as the first argument. There is no provision
to manage build numbers from the command line and other tools
should be used for this (e.g. Artifactory).

## Sample Application

A Visual Studio project is included in this repository under
`sample-sqlite` to test the Nuget package built by this project.

This application does not do anything useful and merely calls
SQLite functions to verify that the package is installed
properly for all platforms and configurations.

[nuget.org]: https://www.nuget.org/packages/StoneSteps.SQLite.Static/
