# This should be included after qtargets.mk
$(USEFILE): $(WFD_SRCDIR)/wfdcfg/usemsg.sh
	DESC="$(DESC)" \
	$(SHELL) "$<" > "$@"
