This is a mirror of the Mercurial repository on Sourceforge for a package
that is a component of the GNV project for OpenVMS.

Its purpose is to provide a record of the source used for the GNV packaged
version of the product.

The original unmodified source is located in the `reference/` directory and
all OpenVMS-specific changes that have not been checked into the parent
repository are located in the `vms_source/` directory.

An OpenVMS concealed logical name of `src_root:` is for the reference directory
and an OpenVMS concealed logical name of `vms_root:` is set to the `vms_source`
directory.

These are kept separate to make it easy to determine what OpenVMS-specific
changes where made and to allow retargeting the `src_root:` to a different
version to easily support maintaining multiple versions such as release,
beta, and trunk at the same time.

The user can then create a logical name of `lcl_root:` for the directory that
will contain the build products.  The `lcl_root:` directory tree is the only
directory tree that the build procedure should be modifying.

A logical name search list of `prj_root:` is set to `lcl_root:,vms_root:,src_root:`
for building the package.

Git was chosen for managing jhamby's fork of this source code repository as there
is now a git client that can execute directly on OpenVMS. This repository
contains branch names of the form `vm.n` where `m` and `n` are respectively
the major and minor version numbers of the releases contained in the branch. The
releases within each branch are tagged with names of the form `vm.n.u(b)` where `m`
and `n` have the same interpretation as for branch names and `u` and `b` are the update
number and build number respectively of the release associated with the tag name.
