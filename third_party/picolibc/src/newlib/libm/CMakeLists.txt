target_include_directories(picolibc PUBLIC common)

target_sources(picolibc PRIVATE common/sf_nan.c)
target_sources(picolibc PRIVATE common/s_nan.c)
target_sources(picolibc PRIVATE common/acosl.c)
target_sources(picolibc PRIVATE
        common/acoshl.c
        common/sincosf.c
        common/sinl.c
        common/sinf.c
        common/exp.c
        common/log.c
        common/log10l.c
        common/s_trunc.c
        common/truncl.c
        common/floorl.c
        common/pow.c
        common/s_pow10.c
        common/fmodl.c
        common/cbrtl.c
        )
target_sources(picolibc PRIVATE
        math/e_acos.c
        math/e_asin.c
        math/e_atan2.c
        math/s_sin.c
        math/s_cos.c
        math/s_tan.c
        math/e_log.c
        math/ef_log.c
        math/e_log10.c
        math/e_exp.c
        math/s_atan.c
        math/e_pow.c
        math/s_fabs.c
        math/e_rem_pio2.c
        math/k_tan.c
        math/s_floor.c
        math/e_pow.c
        math/e_fmod.c
        )

target_compile_definitions(picolibc PRIVATE _IEEE_LIBM)

add_subdirectory(math)
