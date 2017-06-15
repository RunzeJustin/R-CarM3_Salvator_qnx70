define PINFO
PINFO DESCRIPTION="PCI Server HW Dependent Module For Renesas R-Car Gen3 boards"
endef

# Module versioning. By specifying the SO_VERSION, we get DLL major/minor number
# versioning as with other libraries.
# A change to the major number is required if a module API will be broken. A
# change to the minor number MUST be backwards compatible with all lower
# numbered minor version of the same major number. This is used for bug fixes
# and/or additional functionality which does not change behaviour from the
# perspective of the DLL user
#
# IMPORTANT - if you bump the SO_VERSION, do everyone a favour and add comments
#			  to the .use file indicating what changed
SO_VERSION=1.0

# qmacros.mk does not have a VERSION_TAG_DLL macro to enable the versioning of
# DLL's using the SO_VERSION= macro similar to libraries which use the so/
# directory name instead of dll/. We don't want to use the so/ directory name in
# our DLL's because that triggers the creation of static libraries as well.
# Instead we create the necessary macro here perhaps moving it to qmacros.mk
# later
VERSION_TAG_DLL=$(foreach ver,$(firstword $(SO_VERSION) 1),.$(ver))


# Objects not required by this variant that would otherwise be brought in
#
EXCLUDE_OBJS+=

