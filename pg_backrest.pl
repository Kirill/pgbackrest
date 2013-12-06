#!/usr/bin/perl -w

use strict;
use File::Basename;
use Getopt::Long;
use Config::IniFiles;

# Process flags
my $bNoCompression;
my $bNoChecksum;
my $strConfigFile;

GetOptions ("no-compression" => \$bNoCompression,
            "no-checksum" => \$bNoChecksum,
            "config=s" => \$strConfigFile)
    or die("Error in command line arguments\n");

####################################################################################################################################
# TRIM - trim whitespace off strings
####################################################################################################################################
sub trim
{
    my $strBuffer = shift;

	$strBuffer =~ s/^\s+|\s+$//g;

	return $strBuffer;
}

####################################################################################################################################
# EXECUTE - execute a command
####################################################################################################################################
sub execute
{
    my $strCommand = shift;
    
#   print("$strCommand\n");
    my $strOutput = qx($strCommand) or return 0;
#   print("$strOutput\n");
    
    return($strOutput);
}

####################################################################################################################################
# FILE_HASH_GET - get the sha1 hash for a file
####################################################################################################################################
sub file_hash_get
{
    my $strCommand = shift;
    my $strFile = shift;
    
    $strCommand =~ s/\%file\%/$strFile/g;
    
    my $strHash = trim(execute($strCommand));
    
    return($strHash);
}

####################################################################################################################################
# START MAIN
####################################################################################################################################
# Get the command
my $strOperation = $ARGV[0];

####################################################################################################################################
# LOAD CONFIG FILE
####################################################################################################################################
if (!defined($strConfigFile))
{
    $strConfigFile = "/etc/pg_backrest.conf";
}

my %oConfig;
tie %oConfig, 'Config::IniFiles', (-file => $strConfigFile) or die "Unable to find config file";

####################################################################################################################################
# ARCHIVE-LOCAL Command !!! This should become archive-push with no hostname
####################################################################################################################################
if ($strOperation eq "archive-local")
{
    # archive-local command must have three arguments
    if (@ARGV != 3)
    {
        die "not enough arguments - show usage";
    }

    # Get the source dir/file
    my $strSourceFile = $ARGV[1];
    
    unless (-e $strSourceFile)
    {
        die "source file does not exist - show usage";
    }

    # Get the destination dir/file
    my $strDestinationFile = $ARGV[2];

    # Make sure the destination directory exists
    unless (-e dirname($strDestinationFile))
    {
        die "destination dir does not exist - show usage";
    }

    # Make sure the destination file does NOT exist - ignore checksum and extension in case they (or options) have changed
    if (glob("$strDestinationFile*"))
    {
        die "destination file already exists";
    }

    # Calculate sha1 hash for the file (unless disabled)
    if (!$bNoChecksum)
    {
        $strDestinationFile .= "-" . file_hash_get($oConfig{command}{checksum}, $strSourceFile);
    }
    
    # Setup the copy command
    my $strCommand = "";

    if ($bNoCompression)
    {
        $strCommand = $oConfig{command}{copy};
        $strCommand =~ s/\%source\%/$strSourceFile/g;
        $strCommand =~ s/\%destination\%/$strDestinationFile/g;
    }
    else
    {
        $strCommand = $oConfig{command}{compress};
        $strCommand =~ s/\%file\%/$strSourceFile/g;
        $strCommand .= " > $strDestinationFile.gz";
    }
    
    # Execute the copy
    execute($strCommand);

#    print "$strCommandManifest\n";
#    print execute($strCommandManifest) . "\n";
    exit 0;
}

####################################################################################################################################
# GET MORE CONFIG INFO
####################################################################################################################################
if (!defined($oConfig{common}{base_path}))
{
    die 'undefined base path';
}

####################################################################################################################################
# BACKUP
####################################################################################################################################
if ($strOperation eq "backup")
{
    # backup command must have three arguments
    if (@ARGV != 2)
    {
        die "not enough arguments - show usage";
    }

    # Get the cluster name
    my $strCluster = $ARGV[1];

    if (!defined($strCluster))
    {
        die 'undefined cluster';
    }

    if (!defined($oConfig{"cluster:$strCluster"}{pgdata}))
    {
        die 'undefined cluster pgdata';
    }

    my $strCommand = $oConfig{command}{manifest};
    $strCommand =~ s/\%path\%/$oConfig{"cluster:$strCluster"}{pgdata}/g;
    my $strManifest = execute($strCommand);
    
    my @stryFile = split("\n", $strManifest);

    # Flag to only allow the root directory once
    my $bRootDir = 0;
    
    for (my $iFileIdx = 0; $iFileIdx < scalar @stryFile; $iFileIdx++)
    {
        my @stryField = split("\t", $stryFile[$iFileIdx]);
        
        my $dfTime = $stryField[0];
        my $lInode = $stryField[1];
        my $cType = $stryField[2];
        my $strPermission = $stryField[3];
        my $strUser = $stryField[4];
        my $strGroup = $stryField[5];
        my $strSize = $stryField[6];
        my $strName = $stryField[7];

        if (!defined($strName))
        {
            if ($bRootDir)
            {
                die "Root dir appeared twice - check manifest command"
            }
            
            $bRootDir = 1;
            $strName = ".";
        }
        
        # Don't process anything in pg_xlog
        if (index($strName, 'pg_xlog/') != 0)
        {
            # Process directories
            if ($cType eq "d")
            {
                print "dir: $strName\n"
            }

            # Process symbolic links (hard links not supported)
            elsif ($cType eq "l")
            {
                print "link: $strName\n"
            }
        
            # Process files except those in pg_xlog (hard links not supported)
            elsif ($cType eq "f")
            {
                    print "file: $strName\n"
            }

            # Unrecognized type - fail
            else
            {
                die "Unrecognized file type $cType for file $strName";
            }
        }
    }
    
#    print scalar @stryFile . "\n";
}
