# Helper functions for colorized console output.
#
# Written by Andreas Nicolai (andreas.nicolai -at- tu-dresden.de), distributed under the BSD license

import config
from colorama import *

def printError(msg):
	if config.USE_COLORS:
		print Fore.RED + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg

def printWarning(msg):
	if config.USE_COLORS:
		print Fore.YELLOW + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg

def printNotification(msg):
	if config.USE_COLORS:
		print Fore.GREEN + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL
	else:
		print msg
