class Colors:
    PINK = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    CYAN = '\033[33m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    B_ORG = '\033[100m'
    RESET = '\033[0m'

    @staticmethod
    def print_warning(what: str) -> None:
        print(Colors.YELLOW + '  !  ' + what + Colors.RESET)

    @staticmethod
    def print_error_output(what: str) -> None:
        print(Colors.B_ORG + what + Colors.RESET)

    @staticmethod
    def print_section(what: str) -> None:
        print()
        print(Colors.UNDERLINE + Colors.CYAN + Colors.BOLD + what + Colors.RESET)
