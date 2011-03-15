#### Download

I moved these scripts to github some time ago. You can find the
latest version on
<a href="https://github.com/diversen/densite">github.com/diversen/densite</a>
Best option is to cd into your drush/commands folder, and clone the script:

     git clone git://github.com/diversen/densite.git

#### Install

If you clone the scripts into the drush/commands folder
then they are installed. You can use the master or select latest tag.

#### Introduction

The following scripts are tested and working on Debian systems
(Tested on Ubuntu 8.04 Hardy and Ubuntu 8.1, Ubuntu 10.01 and 10.04)
where you are using a Drupal system, or want to install a Drupal system
in a hopefully easy manner. MySQL and Postgresql is supported.
The densite scripts creates virtual host configuration and drupal-5.x and
drupal-6.x sites (most testing is done with drupal-6.x). It also updates
your /etc/hosts file with installed site.

The script also secures your site by setting good permissions for your
file folder and your settings.php file. The default configuration will also
delete all .txt files. Finally crontab will be updated.

#### Requirements

* Access to creating crontab entries
* a2ensite
* a2dissite
* [drush-All-Versions-2.0.tar.gz](http://ftp.drupal.org/files/projects/drush-All-Versions-2.0.tar.gz) Or later.
* Or any later version should work.
* PHP, Apache2, MySQL or PostgreSQL
* sudo (and root privileges) (on other Debian you may need to do

    su root

* Access to creating databases (MySQL or PostgreSQL or both)

#### Config

For configuration you can look at the following two files and change them
as needed. Else they will just be created from the default config files:

    /path/to/drush/commands/densite/apache2/apache2.conf
    /path/to/drush/commands/densite/densite.conf

#### Usage

If you yet does not have a working drupal install, you can use the densitein
(densite install) command which will prompt for a version to
download and install. The version you download will be installed
in your current working directory. E.g. make a www directory in your
home folder, e.g:

    mkdir /home/user/www && cd /home/user/www
    drush densitein www.example.com

If you dont have any real domains to play around with, just use locale IPs like
127.0.0.1, 127.0.0.2 or your computers names.

To create a sub site in a install of drupal. cd to the document root of
your newly installed site, e.g.:

    /home/user/www/www.example.com/htdocs

Create and enable a subsite use:

    drush densite example2.com

The name your are using will be used as the ServerName in your VirtualHost configuration. The name will figure as the site name in the sites/ folder, e.g. sites/dev.os-cms.net for the above example. It will also be used as the apache2 virtualHost configuration placed in /etc/apache2/sites-available folder. If you choose to let the densite script generate a database name, then the name of the database will be devoscmsnet (the site's name without any special signs). Created log files for apache2 will be placed in (from your current drupal document root view):

    ../../dev.os-cms.net/logs
    ../../dev.os-cms.net/logs/access.log
    ../../dev.os-cms.net/logs/error.log

Delete the newly created subsite:

    drush dissite example2.com

Deletes database for website, log files, apache2 virtualhost file and
reloads apache2 and removes the site files, and removes the crontab entry,
step by steep.

In order to create and enable a new (s)tand(a)lone or base install:

cd to document root of your site, e.g:

    cd /home/user/www/www.example.com/htdocs
    drush densitesa example3.com

Notice the name of the command. It is just densite ending with 'sa' (standalone).
 You can clone the current site or you can download a version of
drupal of your choice. You will be prompted for the version you wish to
download. The new site will be created like this (from your current drupal
document root view:)

    ../../dev.os-cms.net/logs
    ../../dev.os-cms.net/htdocs

Notice That a base install creates a folder called htdocs containing the
new base site. Log files will also be created as in the densite command:

    ../../dev.os-cms.net/logs
    ../../dev.os-cms.net/logs/access.log
    ../../dev.os-cms.net/logs/error.log

Delete a (s)tand(a)lone drupal install:

    drush dissitesa example3.com

In order to delete a standalone or base site all sub sites needs to
be deleted first. For each of the standalone code bases you can use the
densite command for creating new sub sites. Or the dissite command for
deleting them.

List all sites installed with the densite, densitesa, and densitein commands.
You have to stand in a directory containing a base install, e.g. in:

    cd /home/user/www/dev.os-cms.net/htdocs

And do a:

    drush listsites

#### Options

You can use the following flags:

    --simulate

flag to see what the commands will do to your system. When using simulate there
will be no changes to your system.

    -f

Force the script to continue when errors are encountered. E.g.:
You have tried to install a site but your failed halfway due to permissions.
You can then change the permissions on your server, and force the script to execute.

    -s

Silence the script will produce less output, and only prompt you when
questions are raised.