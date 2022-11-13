#!/usr/bin/env bash

# must install dnf first
# On Debian, you need to:
# sudo apt install dnf

set -e

function do_bootstrap () {
	echo "STARTING BOOTSTRAP" 2>&1
	local install_root="${1}"

	if [ ! -d "${install_root}" ]; then
		echo "install_root '${install_root}' is not a valid directory."
		exit 1
	fi

	local releasever=36
	local basearch=x86_64

	dnf_config_dir="${install_root}/etc/dnf"
	dnf_config_file="${dnf_config_dir}/dnf.conf"

	resolv_file="${install_root}/etc/resolv.conf"

	mkdir -p "${dnf_config_dir}"

	cat >"${dnf_config_file}" <<-EOF
		[main]
		gpgcheck=1
		installonly_limit=3
		clean_requirements_on_remove=True
		reposdir=

		[fedora]
		name=Fedora $releasever - $basearch
		failovermethod=priority
		metalink=https://mirrors.fedoraproject.org/metalink?repo=fedora-$releasever&arch=$basearch
		enabled=1
		metadata_expire=7d
		gpgcheck=0
		skip_if_unavailable=False
	EOF

	# use hardlink because NetworkManager changes this, and chroot will break symlink
	ln --force /etc/resolv.conf "${resolv_file}"

	sudo dnf install \
		--config=${install_root}/etc/dnf/dnf.conf \
		--installroot=${install_root} \
		--releasever=36 \
		--setopt=install_weak_deps=False \
		--assumeyes \
		--nodocs \
		boost-devel ccache cmake gcc gcc-c++ git ninja-build python3 python3-jinja2 python3-pyelftools python3-toposort python3-pip xorriso

	chroot "${install_root}" /usr/bin/pip3 install "dijkstar==2.6.0"

	mkdir -p "${install_root}/tmp"

	cp "${BASH_SOURCE[0]}" "${install_root}/tmp"

	chroot "${install_root}" "/tmp/${BASH_SOURCE[0]}" setup /root
}

function do_setup () {
	#todo check /etc/os-release; assert_fedora_36
	echo "STARTING SETUP; ASSUMING ROOT OS IS FEDORA WITH REQUISITE PACKAGES" 1>&2
	local repo_dir="${1}"

	if [ ! -d "${repo_dir}" ]; then
		echo "repo_dir '${repo_dir}' is not a valid directory."
		exit 1
	fi

	build_llvm

	cd ${repo_dir}

	# for now
	set +e

	git clone https://github.com/MAYHEM-Lab/lidl
	(
		cd lidl
		git checkout 42dd6b8c6aafbb9d3acf2fa8c8e8c5cbd3d3ff0c
		build_lidl
	)

	git clone https://github.com/MAYHEM-Lab/ambience
	(
		cd ambience
		git checkout ed8469e4152a602c0270869bfcbedefd45dc3947
		build_ambience target
	)

	(
		cd ambience
		build_basic_calc
	)
}

function build_llvm () {
	local llvm_src_dir="/opt/llvm/src"
	local llvm_build_dir="/opt/llvm/src/build"
	local llvm_install_dir="/opt/llvm"

	mkdir -p "${llvm_src_dir}"
	mkdir -p "${llvm_build_dir}"
	mkdir -p "${llvm_install_dir}"

	(
		cd "${llvm_src_dir}"
		git init . -b master
		git pull --depth=1 https://github.com/llvm/llvm-project 1f9140064dfbfb0bbda8e51306ea51080b2f7aac

		cmake -G Ninja -B "${llvm_build_dir}" -S "${llvm_src_dir}/llvm" \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_INSTALL_PREFIX="${llvm_install_dir}" \
			-DLLVM_ENABLE_PROJECTS='clang;lld' \
			-DLLVM_ENABLE_RUNTIMES=''

		cd "${llvm_build_dir}"
		ninja install
	)
}

function build_lidl () {
	git submodule update --init --recursive
	cmake -G Ninja -B target
	cd target
	ninja lidlc
	install src/tools/lidlc /usr/local/bin
}

function build_ambience () {
	local build_dir="${1}"
	cmake -G Ninja -B "${build_dir}" -DTOS_BOARD=x86_64_pc -DBUILD_EXAMPLES=No -DBUILD_TESTS=No
	(
		cd third_party/limine/binaries
		make limine-install
	)
	cd "${build_dir}"
	ninja all
}

function build_basic_calc () {
	# why hosted broke?
	python3 ambience/ambictl/build.py ambience/ambictl/calc_test_deployment
}

if [ $(id -u) -ne 0 ]; then
	echo "script requires root (for dnf and chroot)" 1>&2
	exit 1
fi

ACTION="${1}"

case "${ACTION}" in
	setup )
		WORK_DIR="${2}"
		do_setup "${WORK_DIR}"
		;;
	bootstrap )
		INSTALL_ROOT="${2}"
		do_bootstrap "${INSTALL_ROOT}"
		;;
	* )
		echo "must specify action as arg1 (setup, bootstrap)" 2>&1
		;;
esac

