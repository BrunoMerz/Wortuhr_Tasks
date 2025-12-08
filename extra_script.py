# Building: wordclock

Import("env")

from datetime import datetime
now = datetime.now()
bd = "#define BUILD_DATE \"" + f"{now.strftime('%d.%m.%Y')}" + "\"\n"
bt = "#define BUILD_TIME \"" + f"{now.strftime('%H:%M:%S')}" + "\"\n"
with open("src/BuildDateTime.h", "w") as f:
        f.write(bd)
        f.write(bt)

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

