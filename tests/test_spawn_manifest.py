import json
import os
import subprocess
import sys
from pathlib import Path
import unittest


REPO_ROOT = Path(__file__).resolve().parents[1]
MANIFEST_SCRIPT = REPO_ROOT / "tools" / "extract_spawn_manifest.py"
SNAPSHOT_PATH = REPO_ROOT / "tests" / "expected_spawn_manifest.json"


class SpawnManifestSnapshotTest(unittest.TestCase):
    def _run_manifest(self, extra_env: dict[str, str] | None = None) -> dict:
        env = os.environ.copy()
        if extra_env:
            env.update(extra_env)
        result = subprocess.run(
            [sys.executable, str(MANIFEST_SCRIPT)],
            check=True,
            capture_output=True,
            text=True,
            env=env,
        )
        return json.loads(result.stdout)

    def test_manifest_matches_snapshot(self) -> None:
        current = self._run_manifest()
        expected = json.loads(SNAPSHOT_PATH.read_text(encoding="utf-8"))
        hlil_manifest = current.get("combined", {}).get("hlil", {})
        self.assertGreater(
            len(hlil_manifest),
            0,
            "Parsed HLIL manifest should contain at least one classname",
        )
        self.assertIn(
            "monster_jorg",
            hlil_manifest,
            "Expected monster_jorg entry missing from HLIL manifest",
        )
        self.assertEqual(
            hlil_manifest["monster_jorg"].get("function"),
            "sub_10001ac0",
            "monster_jorg HLIL manifest entry does not match expected function",
        )
        self.assertIn(
            "weapon_rtdu",
            hlil_manifest,
            "Expected weapon_rtdu entry missing from HLIL manifest",
        )
        self.assertEqual(
            hlil_manifest["weapon_rtdu"].get("function"),
            "SpawnItemFromItemlist",
            "weapon_rtdu HLIL manifest entry does not match expected function",
        )
        self.assertEqual(current, expected)

    def test_monster_sentinel_respects_feature_flag(self) -> None:
        default_manifest = self._run_manifest()
        repo_manifest = default_manifest.get("combined", {}).get("repo", {})
        self.assertIn(
            "monster_sentinel",
            repo_manifest,
            "Custom sentinel should be present when the feature flag is enabled",
        )

        parity_manifest = self._run_manifest({"OBLIVION_ENABLE_MONSTER_SENTINEL": "0"})
        parity_repo_manifest = parity_manifest.get("combined", {}).get("repo", {})
        self.assertNotIn(
            "monster_sentinel",
            parity_repo_manifest,
            "Sentinel entry must be suppressed when the feature flag is disabled",
        )
        parity_diff = parity_manifest.get("comparison", {})
        self.assertNotIn(
            "monster_sentinel",
            parity_diff.get("missing_in_hlil", []),
            "Parity manifest diffs should stay clean when sentinel support is disabled",
        )


if __name__ == "__main__":
    unittest.main()
