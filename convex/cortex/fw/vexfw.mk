# List of all the VEX/cortex firmware test files.
VEXFWSRC = ${CONVEX}/fw/vexshell.c \
           ${CONVEX}/fw/vexcortex.c \
           ${CONVEX}/fw/vexdigital.c \
           ${CONVEX}/fw/vexconfig.c \
           ${CONVEX}/fw/vexext.c \
           ${CONVEX}/fw/vexencoder.c \
           ${CONVEX}/fw/vexsonar.c \
           ${CONVEX}/fw/vexmotor.c \
           ${CONVEX}/fw/vexspi.c \
           ${CONVEX}/fw/vexlcd.c \
           ${CONVEX}/fw/vexanalog.c \
           ${CONVEX}/fw/vexctl.c \
           ${CONVEX}/fw/vexime.c \
           ${CONVEX}/fw/vexprintf.c \
           ${CONVEX}/fw/vexaudio.c \
           ${CONVEX}/fw/vexrttl.c \
           ${CONVEX}/fw/vexsensor.c \
           ${CONVEX}/fw/vexbkup.c \
           ${CONVEX}/fw/vextest.c

# Required include directories
VEXFWINC = ${CONVEX}/fw
