from textual.app import ComposeResult
from textual.containers import Container, Horizontal
from textual.screen import Screen, ModalScreen
from textual.binding import Binding
from textual.widgets import DataTable, Footer, Label, Static, OptionList, Input, Button
from textual.widgets.option_list import Option


class BanActionsModal(ModalScreen):
    CSS = """
    BanActionsModal {
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

    def __init__(self, ban_data):
        super().__init__()
        self.ban_data = ban_data

    def compose(self) -> ComposeResult:
        yield Container(
            Static(f"Actions for IP: {self.ban_data[1]}", id="modal_title"),
            OptionList(
                Option("Unban IP", id="unban"),
                Option("Cancel", id="cancel"),
            ),
            id="dialog"
        )

    def on_option_list_option_selected(self, event: OptionList.OptionSelected) -> None:
        self.dismiss(event.option.id)


class AddBanModal(ModalScreen):
    CSS = """
    AddBanModal {
        align: center middle;
    }

    #add_dialog {
        width: 60;
        height: auto;
        border: thick $background 80%;
        background: $surface;
        padding: 2;
    }

    Input {
        margin: 1 0;
    }

    #button_container {
        margin-top: 1;
        height: auto;
        align: center middle;
    }

    Button {
        margin: 0 1;
    }
    """
    BINDINGS = [
        Binding("escape", "dismiss(None)", "Cancel"),
    ]

    def compose(self) -> ComposeResult:
        yield Container(
            Static("Add New Ban", id="modal_title"),
            Input(placeholder="IP Address (e.g., 192.168.0.1)", id="ip_input"),
            Input(placeholder="Reason (optional)", id="reason_input"),
            Horizontal(
                Button("Add Ban", id="btn_add", variant="success"),
                Button("Cancel", id="btn_cancel", variant="error"),
                id="button_container"
            ),
            id="add_dialog"
        )

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn_add":
            ip = self.query_one("#ip_input", Input).value.strip()
            reason = self.query_one("#reason_input", Input).value.strip()

            if ip:
                self.dismiss({"ip": ip, "reason": reason or "No reason provided"})
            else:
                pass
        elif event.button.id == "btn_cancel":
            self.dismiss(None)


class BansManagement(Screen):
    CSS = """
    #title {
        content-align: center middle;
        text-style: bold;
        margin: 1;
        height: 3;
    }

    #controls {
        height: auto;
        padding: 1;
    }
    """
    BINDINGS = [
        Binding("q", "quit", "Quit"),
        Binding("r", "refresh", "Refresh"),
        Binding("a", "add_ban", "Add Ban"),
    ]

    def __init__(self, db_manager):
        super().__init__()
        self.db = db_manager
        self.selected_row = None

    def compose(self) -> ComposeResult:
        yield Static("Bans Management", id="title")
        yield Container(DataTable(id="bans_table"), id="table_container")

        yield Container(
            Label("Total bans: 0 | Click on a row for actions | Press 'a' to add ban", id="selected_info"),
            id="controls"
        )

        yield Footer()

    def on_mount(self) -> None:
        table = self.query_one("#bans_table", DataTable)
        table.cursor_type = "row"
        table.add_columns("ID", "IP Address", "Banned At", "Reason")
        self.load_bans()

    def load_bans(self) -> None:
        table = self.query_one("#bans_table", DataTable)
        table.clear()

        bans = self.db.get_all_bans()
        for ban in bans:
            table.add_row(
                ban["id"], ban["ip_address"], ban["banned_at"], ban["reason"]
            )

        self.query_one("#selected_info", Label).update(
            f"Total bans: {len(bans)} | Click on a row for actions | Press 'A' to add ban"
        )

    def on_data_table_row_selected(self, event: DataTable.RowSelected) -> None:
        self.selected_row = event.row_key
        row_data = event.data_table.get_row(self.selected_row)

        self.run_worker(self.show_actions_modal(row_data), exclusive=True)

    async def show_actions_modal(self, row_data) -> None:
        action = await self.app.push_screen_wait(BanActionsModal(row_data))

        if action == "unban":
            self.action_unban(row_data)

    def action_refresh(self) -> None:
        self.load_bans()
        self.notify("Bans list refreshed!", title="Success")

    def action_unban(self, row_data) -> None:
        ban_id = row_data[0]
        ip_address = row_data[1]

        if self.db.unban_ip(ban_id):
            self.notify(f"IP {ip_address} has been unbanned!", severity="information")
            self.load_bans()
        else:
            self.notify("Failed to unban IP!", severity="error")

    def action_add_ban(self) -> None:
        self.run_worker(self.show_add_ban_modal(), exclusive=True)

    async def show_add_ban_modal(self) -> None:
        result = await self.app.push_screen_wait(AddBanModal())

        if result:
            ip = result["ip"]
            reason = result["reason"]

            if self.db.ban_ip(ip, reason):
                self.notify(f"IP {ip} has been banned!", severity="warning")
                self.load_bans()
            else:
                self.notify(f"IP {ip} is already banned or invalid!", severity="error")

    def action_quit(self) -> None:
        self.app.pop_screen()
