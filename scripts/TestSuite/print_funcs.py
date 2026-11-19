import config

# colorama is an optional dependency (see requirements.txt). Without it the messages are
# printed without colors.
try:
	from colorama import init as _colorama_init, Fore, Style
	_HAVE_COLORAMA = True
except ImportError:
	_HAVE_COLORAMA = False


def initColors():
	"""Initializes colored console output. Returns True if colorama is available."""
	if _HAVE_COLORAMA:
		_colorama_init() # init ANSI code filtering for windows
	return _HAVE_COLORAMA


def printError(msg):
	if config.USE_COLORS and _HAVE_COLORAMA:
		print(Fore.RED + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL)
	else:
		print(msg)

def printWarning(msg):
	if config.USE_COLORS and _HAVE_COLORAMA:
		print(Fore.YELLOW + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL)
	else:
		print(msg)

def printNotification(msg):
	if config.USE_COLORS and _HAVE_COLORAMA:
		print(Fore.GREEN + Style.BRIGHT + msg + Fore.RESET + Style.RESET_ALL)
	else:
		print(msg)
