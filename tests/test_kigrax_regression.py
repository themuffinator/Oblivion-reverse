import re
from pathlib import Path
import unittest

REPO_ROOT = Path(__file__).resolve().parents[1]
KIGRAX_SOURCE = REPO_ROOT / "src" / "game" / "m_kigrax.c"


def extract_function_block(source: str, function_name: str) -> str:
    pattern = re.compile(rf"{function_name}\s*\([^)]*\)\s*\{{", re.MULTILINE)
    match = pattern.search(source)
    if not match:
        raise AssertionError(f"Function {function_name} not found in m_kigrax.c")
    start = match.end()
    depth = 1
    idx = start
    while idx < len(source) and depth > 0:
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
        idx += 1
    return source[match.start():idx]


class KigraxDeathRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source_text = KIGRAX_SOURCE.read_text(encoding="utf-8")

    def test_corpse_switches_to_toss_and_explodes(self) -> None:
        dead_block = extract_function_block(self.source_text, "kigrax_dead")
        self.assertIn(
            "self->movetype = MOVETYPE_TOSS;",
            dead_block,
            "Kigrax corpse must swap to MOVETYPE_TOSS when the death mmove finishes",
        )
        self.assertIn(
            "self->think = kigrax_deadthink;",
            dead_block,
            "Kigrax corpse should install the hover-style explosion thinker",
        )
        self.assertIn(
            "self->nextthink = level.time + FRAMETIME;",
            dead_block,
            "Corpse toss needs to schedule the timed thinker so the explosion triggers",
        )
        self.assertIn(
            "self->timestamp = level.time + 15.0f;",
            dead_block,
            "Corpse explosion timeout deviates from the hover baseline",
        )

        think_block = extract_function_block(self.source_text, "kigrax_deadthink")
        self.assertIn(
            "BecomeExplosion1 (self);",
            think_block,
            "Explosion thinker no longer detonates the corpse",
        )
        self.assertRegex(
            think_block,
            r"level.time\s*\+\s*FRAMETIME",
            "Explosion thinker must keep scheduling itself until the corpse lands",
        )


if __name__ == "__main__":
    unittest.main()
