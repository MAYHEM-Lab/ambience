#!/usr/bin/env bash

# must install dnf first
# On Debian, you need to:
# apt install dnf

set -e


function do_bootstrap () {
	echo "STARTING BOOTSTRAP"
	local releasever=36
	local basearch=x86_64

	dnf_config_dir="${BUILD_OS_ROOT}/etc/dnf"
	dnf_config_file="${dnf_config_dir}/dnf.conf"

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

	dnf install \
		--config=${BUILD_OS_ROOT}/etc/dnf/dnf.conf \
		--installroot=${BUILD_OS_ROOT} \
		--releasever=36 \
		--setopt=install_weak_deps=False \
		--assumeyes \
		--nodocs \
		boost-devel ccache cmake gcc gcc-c++ git ninja-build procps python3 python3-jinja2 python3-pyelftools python3-toposort python3-pip qemu-system-x86 xorriso

	local resolv_file="${BUILD_OS_ROOT}/etc/resolv.conf"

	cat >"${resolv_file}" <<-EOF
		nameserver 1.1.1.1
	EOF


	chroot "${BUILD_OS_ROOT}" /usr/bin/pip3 install "dijkstar==2.6.0"
}

function do_source_install_llvm () {
	local llvm_src_dir="/opt/llvm/src"
	local llvm_build_dir="/opt/llvm/src/build"
	local llvm_install_dir="/opt/llvm"

	mkdir -p "${llvm_src_dir}"
	mkdir -p "${llvm_build_dir}"
	mkdir -p "${llvm_install_dir}"

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
}

function do_source_install_lidl () {
	local lidl_commit=42dd6b8c6aafbb9d3acf2fa8c8e8c5cbd3d3ff0c

	cd /root
	if [ ! -e lidl ]; then
		git clone https://github.com/MAYHEM-Lab/lidl
	fi

	cd lidl
	git checkout $lidl_commit

	git submodule update --init --recursive
	cmake -G Ninja -B target
	cd target

	# don't ninja install b/c of lidl bugs?
	ninja lidlc
	install src/tools/lidlc /usr/local/bin
}

function do_clone_ambience () {
	local ambience_commit=ed8469e4152a602c0270869bfcbedefd45dc3947

	cd /root
	if [ ! -e ambience ]; then
		git clone https://github.com/MAYHEM-Lab/ambience
	fi

	cd ambience
	git checkout $ambience_commit

	(
		cd third_party/limine/binaries
		make limine-install
	)
}

function do_build_basic_calc () {
	cd /root/ambience
	# why hosted broke?
	python3 ambience/ambictl/build.py ambience/ambictl/calc_test_deployment
}

# todo make constands named better. like each side of hostfwd redirect

function do_run_basic_calc () {
	qemu-system-x86_64  \
		-drive file=/tmp/root/mydeployment/cmake-build-barex64/iso/vm-iso.iso,format=raw \
		-serial stdio \
		-display none \
		-no-reboot \
		-no-shutdown \
		-device virtio-net,netdev=calc-net,mac=66:66:66:66:66:66,host_mtu=65535 \
		-netdev user,id=calc-net,hostfwd=udp::1234-:1234
}

function do_query_basic_calc () {
	cd "$(mktemp -d)"
	/usr/local/bin/lidlc -g py -f /root/ambience/ambience/services/interfaces/agent.lidl -o .
	# work around add missing import bug in lidlc generated code
	sed -i '1iimport tos.ae.bench_result' tos/ae/agent.py
	PYTHONPATH="/root/lidl/runtime/python:${PWD}" python3 <<-EOF
		import tos.ae.agent
		import lidlrt
		ip = "127.0.0.1"
		port = 1234
		client = lidlrt.udp_client(tos.ae.agent.agent, (ip, port))
		res = client.start(param=10)
		print(res)
	EOF
}

if [ $(id -u) -ne 0 ]; then
	echo "script requires root (for dnf, chroot, and mount --bind)" 1>&2
	exit 1
fi


SCRIPTPATH="${0}"
SCRIPTNAME="$(basename ${SCRIPTPATH})"
ACTION="${1}"

if [ -z "${AMBIENCE_HELPER_DID_CHROOT}" ]; then
	BUILD_OS_ROOT="$(realpath "${2}")"
	if [ ! -d "${BUILD_OS_ROOT}" ]; then
		echo "BUILD_OS_ROOT '${BUILD_OS_ROOT}' is not a valid directory."
		exit 1
	fi

	if [ "${ACTION}" = "bootstrap" ]; then
		do_bootstrap
		exit
	fi

	# if [ ! -e "${BUILD_OS_ROOT}/proc" ]; then
	# 	mount --bind

	mkdir -p "${BUILD_OS_ROOT}/tmp"
	cp "${SCRIPTPATH}" "${BUILD_OS_ROOT}/tmp/${SCRIPTNAME}"
	AMBIENCE_HELPER_DID_CHROOT=1 chroot "${BUILD_OS_ROOT}" "/tmp/${SCRIPTNAME}" $*
else
	case "${ACTION}" in
		source-install-llvm )
			do_source_install_llvm
			;;
		source-install-lidl )
			do_source_install_lidl
			;;
		clone-ambience )
			do_clone_ambience
			;;
		build-basic-calc )
			do_build_basic_calc
			;;
		setup )
			do_source_install_llvm
			do_source_install_lidl
			do_clone_ambience
			do_build_basic_calc
			;;
		run-basic-calc )
			do_run_basic_calc
			;;
		query-basic-calc )
			do_query_basic_calc
			;;
		run-and-query-basic-calc )
			declare -a KILL_PGIDS
			trap 'kill -- $(for pgid in "${KILL_PGIDS[@]}"; do echo -$pgid; done); echo -e "$STATUS"' EXIT
			set -m # so that each background task gets its own pgid, kill will use the pgid
			coproc do_run_basic_calc
			KILL_PGIDS+=($!)
			until [[ "${line}" =~ "Initialized: vm_priv" ]]; do
				read -u ${COPROC[0]} line
			done
			# Apparently Ambience does lots of setup after its last log
			sleep 5

			do_query_basic_calc &
			query_pid=$!
			KILL_PGIDS+=($query_pid)

			sleep 10 &
			sleep_pid=$!
			KILL_PGIDS+=($sleep_pid)

			echo wait -n $query_pid $sleep_pid -p finished_pid
			wait -p finished_pid -n $query_pid $sleep_pid

			if [ $finished_pid -eq $query_pid ]; then
				STATUS="\n\n\nSUCCESS\n\n\n"
			else
				echo "query timed out" 1>&2
				STATUS="\n\n\nFAILURE\n\n\n"
			fi
			exit
			;;
		* )
			echo "must specify action as arg1" 1>&2
			;;
	esac
fi

