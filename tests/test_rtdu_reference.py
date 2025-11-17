import json
import re
from pathlib import Path
import unittest


def _extract_itemlist_block(source: str) -> str:
    anchor = re.search(r"gitem_t\s+itemlist\[\]\s*=", source)
    if not anchor:
        raise AssertionError("Could not locate gitem_t itemlist[] block in g_items.c")
    brace_start = source.find("{", anchor.end())
    if brace_start == -1:
        raise AssertionError("Could not locate opening brace for itemlist[]")
    depth = 0
    for idx in range(brace_start, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[brace_start:idx]
    raise AssertionError("Failed to locate the end of itemlist[]")


def _extract_initializer(block: str, classname: str) -> str:
    pattern = re.compile(r"\{[^{}]*\"" + re.escape(classname) + r"\"[^{}]*\}", re.S)
    match = pattern.search(block)
    if not match:
        raise AssertionError(f"Could not locate initializer for {classname}")
    return match.group(0)


def _parse_initializer_strings(initializer: str) -> list[str]:
    literals = re.findall(r"\"([^\"]+)\"", initializer)
    if len(literals) < 5:
        raise AssertionError(
            "Expected at least five string literals in the RTDU initializer"
        )
    return literals


def _load_rtdu_hlil_literals(reference_json: Path) -> dict[str, str]:
    entries = json.loads(reference_json.read_text(encoding="utf-8"))
    try:
        start_index = next(
            idx for idx, entry in enumerate(entries) if entry["value"] == "weapon_rtdu"
        )
    except StopIteration as exc:
        raise AssertionError("HLIL strings.json is missing the weapon_rtdu entry") from exc

    expected: dict[str, str] = {"classname": "weapon_rtdu"}
    for entry in entries[start_index + 1 :]:
        category = entry.get("category")
        if category == "sound_path" and "pickup_sound" not in expected:
            expected["pickup_sound"] = entry["value"]
        elif category == "model_path" and "model_path" not in expected:
            expected["model_path"] = entry["value"]
        elif category == "text_label":
            if "icon" not in expected:
                expected["icon"] = entry["value"]
            elif "pickup_text" not in expected:
                expected["pickup_text"] = entry["value"]
        elif category and category.endswith("descriptor"):
            break

        if all(
            key in expected
            for key in ("classname", "model_path", "pickup_sound", "icon", "pickup_text")
        ):
            break

    missing = [
        key
        for key in ("classname", "model_path", "pickup_sound", "icon", "pickup_text")
        if key not in expected
    ]
    if missing:
        raise AssertionError(
            f"HLIL reference JSON missing the following RTDU fields: {', '.join(missing)}"
        )
    return expected


class RTDUReferenceTest(unittest.TestCase):
    def test_weapon_rtdu_initializer_matches_hlil(self) -> None:
        repo_root = Path(__file__).resolve().parents[1]
        g_items_source = (repo_root / "src" / "game" / "g_items.c").read_text(encoding="utf-8")
        itemlist_block = _extract_itemlist_block(g_items_source)
        initializer = _extract_initializer(itemlist_block, "weapon_rtdu")
        literals = _parse_initializer_strings(initializer)

        actual = {
            "classname": literals[0],
            "pickup_sound": literals[1],
            "model_path": literals[2],
            "icon": literals[3],
            "pickup_text": literals[4],
        }

        reference_json = repo_root / "references" / "HLIL" / "oblivion" / "interpreted" / "strings.json"
        expected = _load_rtdu_hlil_literals(reference_json)

        self.assertEqual(
            actual,
            expected,
            "weapon_rtdu initializer no longer matches HLIL references",
        )


if __name__ == "__main__":
    unittest.main()
