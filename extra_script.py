# Building: wordclock70.d1r2.lic.D4
Import("env")
variante=""
defines = env.get("BUILD_FLAGS", [])
print(defines)
if "-D WITH_SECOND_HAND" in defines:
    variante+="_SH"
if "-D WITH_SECOND_BELL" in defines:
    variante+="_SB"
if "-D WITH_ALEXA" in defines:
    variante+="_ALEXA"
if "-D LDR" in defines:
    variante+="_LDR"
if "-D SYSLOGSERVER_PORT=514" in defines:
    variante+="_SYSLOG"
env.Replace(PROGNAME="wordclock%s%s%s%s%s%s%s.%s" % (env.GetProjectOption("custom_version_major"), env.GetProjectOption("custom_version_minor"), variante, env.GetProjectOption("custom_second_hand"), env.GetProjectOption("custom_hardware"), env.GetProjectOption("custom_license"), env.GetProjectOption("custom_pin"), env.GetProjectOption("custom_language")[11:]))

