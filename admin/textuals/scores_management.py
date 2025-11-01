from textual.app import ComposeResult
from textual.containers import Container
from textual.screen import Screen, ModalScreen
from textual.binding import Binding
from textual.widgets import DataTable, Footer, Label, Static, Button, Input
from textual.reactive import reactive

class ScoresManagement(Screen):
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
        Binding("d", "delete_score", "Delete"),
    ]

    def __init__(self, db_manager):
        super().__init__()
        self.db = db_manager
        self.selected_row = None

    def compose(self) -> ComposeResult:
        yield Static("Scores Management", id="title")
        yield Container(DataTable(id="scores_table"), id="table_container")
        yield Container(
            Label("Total scores: 0 | Click on a row and press 'd' to delete | Highest score at top", id="selected_info"),
            id="controls"
        )
        yield Footer()

    def on_mount(self) -> None:
        table = self.query_one("#scores_table", DataTable)
        table.cursor_type = "row"
        table.add_columns("ID", "Player", "Score")
        self.load_scores()

    def load_scores(self) -> None:
        table = self.query_one("#scores_table", DataTable)
        table.clear()
        scores = self.db.get_all_scores()
        scores.sort(key=lambda x: x["score"], reverse=True)
        for score in scores:
            table.add_row(
                score["id"], score["player_id"], score["score"]
            )
        self.query_one("#selected_info", Label).update(
            f"Total scores: {len(scores)} | Click on a row and press 'd' to delete | Highest score at top"
        )

    def on_data_table_row_selected(self, event: DataTable.RowSelected) -> None:
        self.selected_row = event.row_key

    def action_refresh(self) -> None:
        self.load_scores()
        self.notify("Scores list refreshed!", title="Success")

    def action_delete_score(self) -> None:
        if self.selected_row is None:
            self.notify("Please select a score to delete!", severity="warning")
            return

        table = self.query_one("#scores_table", DataTable)
        row_data = table.get_row(self.selected_row)
        score_id = row_data[0]
        player_name = row_data[1]

        if self.db.delete_score(score_id):
            self.notify(f"Score deleted for {player_name}!", severity="information")
            self.load_scores()
        else:
            self.notify("Failed to delete score!", severity="error")

    def action_quit(self) -> None:
        self.app.pop_screen()
