import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
ACTOR_SOURCE = REPO_ROOT / "src" / "game" / "m_actor.c"
SAVE_SOURCE = REPO_ROOT / "src" / "game" / "g_save.c"


def extract_function(source: str, name: str) -> str:
    token = f"{name}"
    search_from = 0
    while True:
        idx = source.find(token, search_from)
        if idx == -1:
            raise AssertionError(f"Could not locate function {name}")

        if idx > 0 and (source[idx - 1].isalnum() or source[idx - 1] == "_"):
            search_from = idx + len(token)
            continue

        cursor = idx + len(token)
        while cursor < len(source) and source[cursor].isspace():
            cursor += 1

        if cursor >= len(source) or source[cursor] != "(":
            search_from = idx + len(token)
            continue

        depth = 1
        cursor += 1
        while cursor < len(source) and depth > 0:
            if source[cursor] == "(":
                depth += 1
            elif source[cursor] == ")":
                depth -= 1
            cursor += 1

        if depth != 0:
            raise AssertionError(f"Unbalanced parentheses in {name}")

        while cursor < len(source) and source[cursor].isspace():
            cursor += 1

        if cursor >= len(source) or source[cursor] != "{":
            search_from = idx + len(token)
            continue

        start = source.rfind("\n", 0, idx) + 1
        depth = 0
        end = cursor
        while end < len(source):
            if source[end] == "{":
                depth += 1
            elif source[end] == "}":
                depth -= 1
                if depth == 0:
                    return source[start:end + 1]
            end += 1

        raise AssertionError(f"Unbalanced braces in {name}")


class ActorPathStateMachineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = ACTOR_SOURCE.read_text(encoding="utf-8")

    def test_spawn_initialises_path_fields(self) -> None:
        body = extract_function(self.source, "Actor_SpawnOblivion")
        for token in (
            "self->oblivion.path_wait_time = -1.0f;",
            "self->oblivion.path_state = ACTOR_PATH_STATE_IDLE;",
            "VectorClear(self->oblivion.path_dir);",
            "self->think = Actor_PathThink;",
        ):
            with self.subTest(token=token):
                self.assertIn(token, body)

    def test_use_schedules_think_and_releases_hold(self) -> None:
        body = extract_function(self.source, "Actor_UseOblivion")
        self.assertIn("self->think = Actor_PathThink;", body)
        self.assertRegex(
            body,
            r"self->oblivion\.path_toggle\s*=\s*0;\s*\n\s*self->oblivion\.path_state\s*=\s*ACTOR_PATH_STATE_SEEKING;",
        )

    def test_target_touch_delegates_to_path_machine(self) -> None:
        body = extract_function(self.source, "target_actor_touch")
        self.assertIn("Actor_PathReached(other, self, next_target);", body)
        self.assertNotIn("G_UseTargets (self, other);", body)

    def test_path_think_reacquires_controller(self) -> None:
        body = extract_function(self.source, "Actor_PathThink")
        self.assertIn("g_edicts[self->oblivion.controller_serial]", body)
        self.assertIn("Actor_AttachController(self, candidate);", body)

    def test_path_reached_honours_waits(self) -> None:
        body = extract_function(self.source, "Actor_PathReached")
        self.assertRegex(
            body,
            r"override_wait\s*=\s*self->oblivion\.path_wait_time;",
        )
        self.assertRegex(
            body,
            r"self->oblivion\.path_state\s*=\s*ACTOR_PATH_STATE_WAITING;",
        )

    def test_save_table_includes_path_fields(self) -> None:
        save_text = SAVE_SOURCE.read_text(encoding="utf-8")
        for entry in (
            "mission_path_state",
            "mission_path_toggle",
            "mission_path_velocity",
        ):
            with self.subTest(entry=entry):
                self.assertIn(entry, save_text)


if __name__ == "__main__":
    unittest.main()
