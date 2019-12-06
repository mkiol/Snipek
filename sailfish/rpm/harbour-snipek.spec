# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       harbour-snipek

# >> macros
%define __provides_exclude_from ^%{_datadir}/.*$
# << macros

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Snipek
Version:    2.0.0
Release:    1
Group:      Qt/Qt
License:    MPLv2.0
URL:        https://github.com/mkiol/Snipek
Source0:    %{name}-%{version}.tar.bz2
Source100:  harbour-snipek.yaml
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   curl
Requires:   binutils
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(commhistory-qt5)
BuildRequires:  desktop-file-utils

%description
Voice assistant for Sailfish OS based on Snips software


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qtc_qmake5 

%qtc_make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
# >> files
%defattr(755,root,root,-)
%{_datadir}/%{name}/snips/*.sh
# << files
