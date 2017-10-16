%define __spec_install_post %{nil}
%define debug_package %{nil}
%define _unpackaged_files_terminatei_build 0
%define build_timestamp %(date +"%Y%m%d") 

Summary:      PHP Extension to disable eval
Name:         php56-diseval
Version:      0.1
Release:      %{build_timestamp}%{?dist}
License:      PHP
Group:        Development/Languages
URL:          https://github.com/mk-j/PHP_diseval_extension
Source:       diseval-%{version}.tar.gz
BuildRequires: php56-devel
Requires: php56-common
Conflicts: php55-diseval
Conflicts: php70-diseval
Conflicts: php71-diseval
Conflicts: php72-diseval

%description
Simple extension to disable eval,create_function, and regex /e modifiers

%prep 
%setup -q -n diseval-%{version}

cat >diseval.ini << 'EOF'
; ----- Enable openssl tools extension module
extension=diseval.so
EOF


%build
%{_bindir}/phpize
%configure --with-php-config=%{_bindir}/php-config
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}

# Drop in the bit of configuration
install -D -m 644 diseval.ini %{buildroot}%{_sysconfdir}/php/php.d/diseval.ini

%files
%{_libdir}/php/modules/diseval.so
%config(noreplace) %{_sysconfdir}/php/php.d/diseval.ini

%changelog
* Fri Oct 6 2017 mk-j <mark@zedwood.com> - 0.1
- initial RPM

