$Id: README.txt,v 1.4 2009/06/25 01:36:30 deniver Exp $

/* vim: set linebreak: */

README.txt

The following scripts are tested and working on Debian (Tested on Ubuntu 8.04 Hardy and Ubuntu 8.1 intreped) where you want to install a drupal system in a hopefully easy manner. The densite scripts also makes it easy for you to create new sub-sites and base-sites. MySQL and Postgresql are supported. The densite scripts creates virtual host configuration and drupal-5.x and drupal-6.x sites (most testing is done with drupal-6.x). The script also secures your site by setting good permissions for your file folder and your settings.php file. The default configuration will also delete all .txt files. Finally crontab will be updated. I am working on making drupal-7.x work as well, but this is still 'work in progress'. 

If you use any of the densite install commands then you will be prompted if you wish to download some default sql files. These dumps can only be used if apache2 can manage Clean URLs, because the dumps were saved when clean URLs were enabled. Otherwise you can just create your own with the drush sql dump, or mysqldump, or pg_dump or any other way, and place them in the densite/sql directoy.  

If you don't want to use sql dumps files you must proceed with manual web install. 

REQUIREMENTS
* a2ensite
* a2dissite
* drush-All-Versions-2.0.tar.gz (or later) http://ftp.drupal.org/files/projects/drush-All-Versions-2.0.tar.gz
* PHP, Apache2, MySQL or PostgreSQL
* sudo (and root privileges)
* Access to creating databases (MySQL or PostgreSQL or both)
* Access to creating crontab entries

INSTALL

with drush:

cd /path/to/drush/commands
drush dl densite

Or manual

tar xvfz densite.tar.gz
mv densite /path/to/drush/commands

For configuration you can look at the following two files and change them as needed. They are not included, but any command will complain if they are missing. The scripts will also ask you if thoses scripts should be created when you run any densite command. You should just say [Y]es to these questions. 

The configuration for apache and for the densite can be found at these locations:  

/path/to/drush/commands/densite/apache2/apache2.conf
/path/to/drush/commands/densite/densite.conf

Depending on your system the densite commands may run without any configuration changes, but it would be a good idea just to look them through. 

USAGE:

If you yet do not not have a working drupal install, you can use the densitein (densite install) command which will prompt for a version to download and install. The version you download will be installed in your current working directory. E.g. make a www directory in your home folder, e.g:

mkdir /home/user/www && cd /home/user/www
drush densitein dev.os-cms.net

If you dont have any real domains to play around with, just use locale IPs like 127.0.0.2, 127.0.0.3, or your computers names. 

To create a sub site in a install of drupal. 
cd to the document root of your newly installed site, e.g. 

/home/user/www/www.os-cms.dk/htdocs 

Create and enable a subsite use:

drush densite dev.os-cms.net

The name your are using will be used as the ServerName in your VirtualHost configuration. The name will figure as the site name in the sites/ folder, e.g. sites/dev.os-cms.net for the above example. It will also be used as the apache2 virtualHost configuration placed in /etc/apache2/sites-available folder. If you choose to let the densite script generate a database name, then the name of the database will be devoscmsnet (the site's name without any special signs). Created log files for apache2 will be placed in (from your current drupal document root view):

../../dev.os-cms.net/logs
../../dev.os-cms.net/logs/access.log
../../dev.os-cms.net/logs/error.log

Delete the newly created subsite:

drush dissite dev.os-cms.net

Deletes database for website, log files, apache2 virtualhost file and reloads apache2 and removes the site files, and removes the crontab entry, step by steep

In order to create and enable a new (s)tand(a)lone or base install:

cd to document root of your site, e.g. 
/home/user/www/www.os-cms.net/htdocs 

drush densitesa dev.os-cms.net

Notice the name of the command. It is just densite ending with 'sa' (standalone). You can clone the current site or you can download a version of drupal of your choice. You will be prompted for the version you wish to download. The new site will be created like this (from your current drupal document root view:)

../../dev.os-cms.net/logs
../../dev.os-cms.net/htdocs

Notice That a base install offcause creates a folder called htdocs containing the new base site. Log files will also be created as in the densite command:

../../dev.os-cms.net/logs
../../dev.os-cms.net/logs/access.log
../../dev.os-cms.net/logs/error.log

Delete a (s)tand(a)lone drupal install:

drush dissitesa dev.os-cms.net

In order to delete a standalone or base site all sub sites needs to be deleted first. For each of the standalone code bases you can use the densite command for creating new sub sites. Or the dissite command for deleting them. 

list all sites installed with the densite, densitesa, and densitein commands. You have to stand in a directory containing a base install, e.g. in:

/home/user/www/dev.os-cms.net/htdocs

do a:

drush listsites

OPTIONS:

You can use the following flags:
--simulate: flag to see what the commands will do to your system. When using 
simulate there will be no changes to your system. 
-f: Force the script to continue when errors are encountered. E.g.: You have tried to install a site but your failed halfway due to permissions. You can then change the permissions on your server, and force the script to execute.   
-s: Silence the script will produce less output, and only prompt you when 
questions are raised.

