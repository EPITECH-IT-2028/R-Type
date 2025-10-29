from textual.app import ComposeResult
from textual.containers import Container
from textual.screen import Screen, ModalScreen
from textual.binding import Binding
from textual.widgets import DataTable, Footer, Label, Static, OptionList
from textual.widgets.option_list import Option

class PlayerActionsModal(ModalScreen):
    CSS = """
    PlayerActionsModal {
        align: center middle;
    }

    #dialog {
        width: 50;
        height: auto;
        border: thick $background 80%;
        background: $surface;
        padding: 1;
    }
    """
    BINDINGS = [
        Binding("escape", "dismiss('cancel')", "Close"),
    ]

    def __init__(self, player_data):
        super().__init__()
        self.player_data = player_data

    def compose(self) -> ComposeResult:
        yield Container(
            Static(f"Actions for: {self.player_data[1]}", id="modal_title"),
            OptionList(
                Option("Ban IP", id="ban"),
                Option("Delete player", id="delete"),
                Option("Cancel", id="cancel"),
            ),
            id="dialog"
        )

    def on_option_list_option_selected(self, event: OptionList.OptionSelected) -> None:
        self.dismiss(event.option.id)

class PlayersManagement(Screen):
    CSS = """
    #title {
        content-align: center middle;
        text-style: bold;
        margin: 1;
        height: 3;
    }

    #button_row {
        height: auto;
        width: 100%;
    }

    #button_row Button {
        margin: 0 1;
        min-width: 15;
    }

    #controls {
        height: auto;
        padding: 1;
    }

    """
    BINDINGS = [
        Binding("q", "quit", "Quit"),
        Binding("r", "refresh", "Refresh"),
    ]

    def __init__(self, db_manager):
        super().__init__()
        self.db = db_manager
        self.selected_row = None

    def compose(self) -> ComposeResult:
        yield Static("Players Management", id="title")
        yield Container(DataTable(id="players_table"), id="table_container")

        yield Container(
            Label("Total players: 0 | Click on a row to open actions menu", id="selected_info"),
            id="controls"
        )

        yield Footer()

    def on_mount(self) -> None:
        table = self.query_one("#players_table", DataTable)
        table.cursor_type = "row"
        table.add_columns("ID", "Username", "IP Address", "Created At", "Status")
        self.load_players()

    def load_players(self) -> None:
        table = self.query_one("#players_table", DataTable)
        table.clear()

        players = self.db.get_all_players()
        for player in players:
            table.add_row(
                player["id"], player["username"], player["ip_address"],
                player["created_at"], player["status"]
            )

    def on_data_table_row_selected(self, event: DataTable.RowSelected) -> None:
        self.selected_row = event.row_key
        row_data = event.data_table.get_row(self.selected_row)

        self.run_worker(self.show_actions_modal(row_data), exclusive=True)

    async def show_actions_modal(self, row_data) -> None:
        action = await self.app.push_screen_wait(PlayerActionsModal(row_data))

        if action == "ban":
            self.action_ban(row_data)
        elif action == "delete":
            self.action_delete(row_data)

    def action_refresh(self) -> None:
        self.load_players()
        self.notify("Players list refreshed!", title="Success")

    def action_delete(self, row_data) -> None:
        player_id = row_data[0]
        username = row_data[1]

        if self.db.delete_player(player_id):
            self.notify(f"Player {username} deleted!", severity="information")
            self.load_players()
        else:
            self.notify("Failed to delete player!", severity="error")

    def action_ban(self, row_data) -> None:
        ip_address = row_data[2]

        if self.db.ban_ip(ip_address, "Banned via admin panel"):
            self.notify(f"IP {ip_address} has been banned!", severity="warning")
        else:
            self.notify(f"IP {ip_address} is already banned!", severity="warning")

    def action_quit(self) -> None:
        self.app.pop_screen()
