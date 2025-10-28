from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, Button, Static
from textual.containers import Container, Vertical
from textual.binding import Binding
from database import DatabaseManager
from textuals import PlayersManagement


class AdminPanel(App):
    CSS = """
    #title {
        content-align: center middle;
        text-style: bold;
        color: $accent;
        margin: 1;
        height: 3;
    }

    #menu_container {
        align: center middle;
        width: 50;
        height: auto;
    }

    Button {
        width: 100%;
        margin: 1;
    }
    """

    BINDINGS = [
        Binding("q", "quit", "Quit", show=True),
    ]

    def __init__(self):
        super().__init__()
        self.db = DatabaseManager()

    def on_mount(self) -> None:
        if not self.db.connect():
            self.notify("Failed to connect to database!", severity="error")
            self.exit()
        else:
            self.notify("Connected to database!", severity="information")

    def compose(self) -> ComposeResult:
        yield Header()
        yield Static("R-Type Admin Panel", id="title")
        yield Container(
            Vertical(
                Button("Manage Players", id="btn_players", variant="primary"),
                Button("Manage Bans", id="btn_bans", variant="warning"),
                Button("Exit", id="btn_exit", variant="error"),
                id="menu_container"
            )
        )
        yield Footer()

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn_players":
            self.push_screen(PlayersManagement(self.db))
        # elif event.button.id == "btn_bans":
            # self.push_screen(BansScreen(self.db))
        elif event.button.id == "btn_exit":
            self.exit()

    def on_unmount(self) -> None:
        self.db.close()


if __name__ == "__main__":
    app = AdminPanel()
    app.run()
