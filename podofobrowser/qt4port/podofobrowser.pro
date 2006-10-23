SUBDIRS += src
TEMPLATE = subdirs 
CONFIG += debug \
          warn_on \
          qt \
          thread 

browser.path = /usr/local/bin/
browser.files = ./bin/podofobrowser

INSTALLS += browser

