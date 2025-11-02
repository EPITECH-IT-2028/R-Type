from typing import Dict, List, Optional
import sqlite3

class DatabaseManager:
    def __init__(self, db_path: str = "../rtype.db"):
        self.db_path = db_path
        self.connection = None

    def connect(self):
        try:
            self.connection = sqlite3.connect(self.db_path)
            self.connection.row_factory = sqlite3.Row
            return True
        except sqlite3.Error as e:
            print(f"Database connection error: {e}")
            return False

    def close(self):
        if self.connection:
            self.connection.close()

    def get_all_players(self) -> List[Dict]:
        try:
            if not self.connection:
                raise Exception("Database not connected")
            cursor = self.connection.cursor()
            cursor.execute("SELECT id, username, ip_address, created_at, is_online FROM players")
            rows = cursor.fetchall()

            return [{
                "id": row["id"],
                "username": row["username"],
                "ip_address": row["ip_address"],
                "created_at": row["created_at"],
                "status": "Online" if row["is_online"] else "Offline"
            } for row in rows]
        except sqlite3.Error as e:
            print(f"Error retrieving players: {e}")
            return []

    def get_player_by_id(self, player_id: int) -> Optional[Dict]:
        if not self.connection:
            raise Exception("Database not connected")
        cursor = self.connection.cursor()
        cursor.execute("SELECT id, username, ip_address, created_at, is_online FROM players WHERE id = ?", (player_id,))
        row = cursor.fetchone()

        if row:
            return {
                "id": row["id"],
                "username": row["username"],
                "ip_address": row["ip_address"],
                "created_at": row["created_at"],
                "status": "Online" if row["is_online"] else "Offline"
            }
        else:
            return None

    def delete_player(self, player_id: int) -> bool:
        if not self.connection:
            raise Exception("Database not connected")
        cursor = self.connection.cursor()
        try:
            cursor.execute("DELETE FROM players WHERE id = ?", (player_id,))
            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Error deleting player: {e}")
            return False

    def get_all_bans(self) -> List[Dict]:
        try:
            if not self.connection:
                raise Exception("Database not connected")
            cursor = self.connection.cursor()
            cursor.execute("SELECT id, ip_address, banned_at, reason FROM bans")
            rows = cursor.fetchall()

            return [{
                "id": row["id"],
                "ip_address": row["ip_address"],
                "banned_at": row["banned_at"],
                "reason": row["reason"]
            } for row in rows]
        except sqlite3.Error as e:
            print(f"Error retrieving bans: {e}")
            return []

    def ban_ip(self, ip_address: str, reason: str = "No reason provided") -> bool:
        if not self.connection:
            raise Exception("Database not connected")
        cursor = self.connection.cursor()
        try:
            cursor.execute("INSERT INTO bans (ip_address, reason) VALUES (?, ?)", (ip_address, reason))
            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Error banning IP: {e}")
            return False

    def unban_ip(self, ban_id: int) -> bool:
        if not self.connection:
            raise Exception("Database not connected")
        cursor = self.connection.cursor()
        try:
            cursor.execute("DELETE FROM bans WHERE id = ?", (ban_id,))
            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Error unbanning IP: {e}")
            return False

    def get_all_scores(self) -> List[Dict]:
        try:
            if not self.connection:
                raise Exception("Database not connected")
            cursor = self.connection.cursor()
            cursor.execute("""
                SELECT scores.id, players.username, scores.score
                FROM scores
                JOIN players ON scores.player_id = players.id
            """)
            rows = cursor.fetchall()

            return [{
                "id": row["id"],
                "player_id": row["username"],
                "score": row["score"],
            } for row in rows]
        except sqlite3.Error as e:
            print(f"Error retrieving scores: {e}")
            return []

    def delete_score(self, score_id: int) -> bool:
        if not self.connection:
            raise Exception("Database not connected")
        cursor = self.connection.cursor()
        try:
            cursor.execute("DELETE FROM scores WHERE id = ?", (score_id,))
            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Error deleting score: {e}")
            return False
