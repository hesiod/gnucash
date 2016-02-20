# GnuCash 2.6.x

Visit the [GnuCash Home Page](http://www.gnucash.org), and
for a jump-start download the latest GnuCash version from
our [download page](http://www.gnucash.org/download)!

The last known stable series is the 2.6.0 series.

# Overview

GnuCash is a personal finance manager. A check-book like register GUI
allows you to enter and track bank accounts, stocks, income and even
currency trades. A full set of reports allow you to see the state of
your finances. The interface is designed to be simple and easy to use,
but is backed with double-entry accounting principles to ensure
balanced books.

## Features

GnuCash features include:

### Accounting

##### Double Entry Accounting
Every transaction must debit one account and credit
another by an equal amount.  This ensures that the "books
balance": that the difference between income and outflow exactly
equals the sum of all assets, be they bank, cash, stock or other.

##### Split Transactions
A single transaction can be split into several
pieces to record taxes, fees, and other compound entries.

##### Scheduled Transactions
GnuCash has the ability to automatically
create and enter transactions, remind when a transaction is due,
give a choice of entering a transaction or postponing it and
remove an automated transaction after a certain period.

##### Income/Expense Account Types (Categories)
These serve not only to
categorize your cash flow, but when used properly with the
double-entry feature, these can provide an accurate Profit&Loss
statement.

##### Chart of Accounts
A master account can have a hierarchy of detail
accounts underneath it.  This allows similar account types
(e.g. Cash, Bank, Stock) to be grouped into one master account
(e.g. Assets).

##### General Ledger
Multiple accounts can be displayed in one register
window at the same time.  This can ease the trouble of tracking
down typing/entry errors.  It also provides a convenient way of
viewing a portfolio of many stocks, by showing all transactions in
that portfolio.

### Easing Day-to-Day tasks

##### An easy-to-use interface
If you can use the register in the back
of your checkbook, you can use GnuCash.  Type directly into the
register, tab between fields, and use quick-fill to automatically
complete the transaction.

##### Mortgage & Loan Repayment Assistant
Used to setup a variable
payment loan scheduled transaction.

##### Reconciliation
Reconcile window with running reconciled and cleared balances
makes reconciliation easy.

##### Small Business Accounting Features
GnuCash can be used
for Customer and Vendor tracking, Invoicing and Bill Payment,
and using different Tax and Billing	Terms in a small business.


### Reports

##### Reports
Display Balance Sheet, Profit & Loss, Portfolio Valuation,
Transaction Reports, or account balance tracking, or export them
as HTML.

##### Custom Reports
If you don't like the provided reports, you can write your own!
If you know a little Scheme, report generation is straightforward.
Reports can be run over any arbitrary period.

### Interoperability and Storage

##### OFX Import
GnuCash can import downloaded OFX/QFX files and
retrieve account info and transactions via OFXDirect. The results
are passed through a transaction matching system that accurately
picks duplicate transactions and assigns contra accounts based on
similar previously-imported transactions.

##### Quicken File Import
Import Quicken QIF style files.  QIF files
are automatically merged to eliminate duplicate transactions.

##### HBCI/FinTS Support
GnuCash also supports the German Home Banking
Computer Interface protocol which includes statement download,
initiate bank transfers and direct debits.

##### SQL Support
SQL-based storage is supported using MySQL, Postgresql, and
SQLite3. Note that this does not support multiple concurrent
access.

### Securities and Stock features

##### Stock/Mutual Fund Portfolios
Track stocks individually (one per
account) or in portfolio of accounts (a group of accounts that can
be displayed together).

##### Stock and Fund Quote Retrieval
Get Stock & Mutual Fund quotes from various web sites, update
portfolio automatically (more funds being added regularly).

##### Currency Managment
Multiple currencies are
supported and can be bought and sold (traded).  Currency movements
between accounts are fully balanced when double-entry is enabled.

##### Multi-currency Transaction Handling
Handle cross-currency transactions with ease!
GnuCash no longer
requires separate currency exchange accounts to handle multiple
currency transfers.

### Awesome Geeky Features

##### Guile extensions
Written in C++ with embedded scheme support via Guile.

##### File Locking
File access is locked in a network-safe fashion, preventing
accidental damage if several users attempt to access the same
file, even if the file is NFS-mounted.

# GnuCash for Users
## Downloads

GnuCash sources and Mac and Windows binaries are hosted at
SourceForge. Links for the current version are provided at
[GnuCash home page](http://www.gnucash.org). We depend upon distribution packagers for
GNU/Linux and *BSD binaries, so if you want a more recent version than
your distribution provides you'll have to build from source.


## Supported Platforms

GnuCash 2.x is known to work with the following operating systems:


| Operating System | Platform |
| ----------|-----------------|
| GNU/Linux | x86, Sparc, PPC |
| FreeBSD   | x86             |
| OpenBSD   | x86             |
| Mac OS X  | x86 and PPC (10.5 and later) |

GnuCash can probably be made to work on any platform for which Gtk+ can,
given sufficient effort. If you try and encounter difficulty, please join
the [developer's mailing list](mailto:gnucash-devel@gnucash.org).


## Runtime Dependencies

The following packages are required to be installed to run GnuCash:

Please consult the [the dependencies README](README.dependencies).

The optional online stock and currency price retrieval feature requires Perl. This is generally already installed on Gnu/Linux and *BSD, and MacOSX.

In addition, some perl modules need to be installed. You can run the
script `update-finance-quote` as root to obtain the latest versions of
required packages. This program requires an C compiler, which is
generally already installed in GNU/Linux and *BSD, but must be
installed separately on Mac OS X; on versions 10.6 and earlier, install
Xcode from the distribution DVD or by downloading from
the [Apple Developer Program](http://developer.apple.com) (you'll need to log in with your Apple ID);
for 10.7 and 10.8, you can get Xcode for free from the App store. 10.9
has a cool feature that detects an attempt to compile and offers to
install the command-line build tools for you.

Microsoft Windows users can use the "Install Online Quotes" program in the Start menu's Gnucash group.

To use the OFX and HBCI import features you need to obtain
the following;
  - [libofx](http://sourceforge.net/projects/libofx/): This library provide support for OFX file imports.
    GnuCash-2.0.0 and newer needs at least the version libofx-0.7.0
    or newer.
  - [aqbanking](http://www2.aquamaniac.de/sites/download/packages.php): This
    library provide support for HBCI and openOFX online actions.
    Three libraries, Ktoblzcheck, Gwenhywfar, and AQBanking, are required.

## Running

For GnuCash invocation details, see the [manpage](doc/gnucash.1).
You can also run `gnucash --help` for the command line options.

You can start GnuCash at the command-line, with `gnucash` or `gnucash <filename>`,
where <filename> is a GnuCash account file.  Sample
accounts can be found in the [`doc/examples` subdirectory](doc/examples).
`*.gnucash` files are GnuCash accounts that can be opened with the "Open File" menu entry.
`*.qif` files are Quicken Import Format files that can be opened with the
"Import QIF" menu entry.

GnuCash responds to the following environment variables:

 - `GNC_BOOTSTRAP_SCM` - the location of the initial bootstrapping scheme code.

 - `GUILE_LOAD_PATH` - an override for the GnuCash load path, used when
  loading scheme files.  It should be a string in the same form as the
  `PATH` or `LD_LIBRARY_PATH` environment variable.

 - `GNC_MODULE_PATH` - an override for the GnuCash load path, used when
  loading gnucash modules.  It should be a string representing a
  proper scheme list.  It should be a string in the same form as the
  `PATH` or `LD_LIBRARY_PATH` environment variable.

 - `GNC_DEBUG` - enable debugging output.  This allows you to turn on
  debugging earlier in the startup process than you can with `--debug`.


## Internationalization

Message catalogs exist for many different languages. In general
GnuCash will use the locale configured in the desktop environment if
we have a translation for it, but this may be overridden if one
likes. Instructions for overriding the locale settings may be found at
the [GnuCash Wiki](http://wiki.gnucash.org/wiki/Locale_Settings)

# Technical Details

### Retrieving the Source Code using Git

We maintain a mirror of
our master repository [on Github](https://github.com/Gnucash/gnucash).
Clone URIs are
on that page, or if you have a Github account you can fork it
there. Note, however, that we do *not* accept Github pull requests:
All patches should be submitted via Bugzilla, see below.

## Building and Installing GnuCash from Source

(For additional build system details, see the [build system README](doc/README.build-system).)

GnuCash uses GNU Automake to handle the build process, so for most of
the details, see the generic instructions in [`INSTALL`](INSTALL).
(If you are building directory from Git,
read the [`README.git`](README.git) file for more instructions.)
Below we detail the GnuCash specific bits.

### Build Dependencies

Prior to building GnuCash, you will have to obtain and install the
following packages:

 - `autoconf`, `automake`, and `libtool`: Packages widely available
   for most distributions.

 - Gnome development system: headers, libraries, etc.

 - [libxml2](http://xmlsoft.org)

 - [SWIG](http://www.swig.org): 2.0.10 or later is needed.

What you'll need to get and install in order to make sure you have all
of these pieces properly installed for your particular operating
system flavor will vary. In general, have a look at the dependencies listed
in [the dependencies README](README.dependencies). Additionally, here's
at least a partial list of what
you'll need for the systems we know about:

For Debian GNU/Linux:
  - `libgnome-dev`
  - `libwebkit-dev`
  - `guile1.8`
  - `libguile9-dev`
  - `libguile9-slib`

### Configuring

GnuCash understands a few non-standard `./configure` options.  You
should run `./configure --help` for the most up to date summary of the
supported options.

If you only want a particular language installed, you can set the
`LINGUAS` environment variable before you run configure. For example,
to only install the French translations, run

```
$ export LINGUAS=fr
$ ./configure
```

If you want to make sure that all languages get installed, run

```
$ unset LINGUAS
$ ./configure
```

Note that while you need the Gnome libraries installed, you don't
need to have a Gnome desktop.

Runtime and install destinations are separate.  The `--prefix` you
specify to configure determines where the resulting binary will look
for things at runtime.  Normally this determines where a `make install`
will put all the files.  However, `automake` also supports the
variable.  `DESTDIR` is used during the `make install` step to relocate
install objects into a staging area.  Each object and path is prefixed
with the value of `DESTDIR` before being copied into the install area.
Here is an example of typical `DESTDIR` usage:

```
make DESTDIR=/tmp/staging install
```

   This places install objects in a directory tree built under
`/tmp/staging`.  If `/gnu/bin/foo` and `/gnu/share/aclocal/foo.m4` are
to be installed, the above command would install
`/tmp/staging/gnu/bin/foo` and `/tmp/staging/gnu/share/aclocal/foo.m4`.

`DESTDIR` can be helpful when trying to build install images and
packages.

#### Handling Inconsistent Gnome Directories
If you have installed different parts of Gnome in different
places (for instance, if you've installed webkit in `/usr/local`) you
will need to set the environment variables `GNOME_PATH` and
`GNOME_LIBCONFIG_PATH`.  See the manpage for gnome-config for more
details.

## Developing GnuCash

### General Information
Before you start developing GnuCash, you should do the following:

1. Read the Wiki page on [GnuCash Development](http://wiki.gnucash.org/wiki/Development)

2. Look over the [doxygen-generated documentation](http://code.gnucash.org/docs/HEAD)

3. Go to the GnuCash website and skim the archives of the GnuCash
   [development mailing list](mailto:gnucash-devel@gnucash.org).

4. Join the GnuCash [development mailing list](mailto:gnucash-devel@gnucash.org). See the [GnuCash website](http://gnucash.org)
   for details on how to do this.

### Submitting Patches

Patches should be created from a git clone using the appropriate
branch HEAD. For those unfamiliar with git, instructions on making a
patch may be found [in the Wiki](http://wiki.gnucash.org/wiki/Git#Patches).

Please attach patches to the appropriate bug or enhancement request
in [Bugzilla](https://bugzilla.gnome.org). Create a
new bug if you don't find one that's applicable. Please don't submit
patches to either of the mailing lists, as they tend to be
forgotten.

Thank you.
