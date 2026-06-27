import json
import tempfile
import unittest
from pathlib import Path

from scripts.collect_knowledge_candidates import collect_candidates
from scripts.evaluate_promotion import evaluate_candidate


class KnowledgePromotionTest(unittest.TestCase):
    def test_collector_preserves_source_and_does_not_approve(self):
        with tempfile.TemporaryDirectory() as tmp:
            change = Path(tmp) / "change"
            output = Path(tmp) / "knowledge/candidates"
            source = change / "evidence/knowledge-candidates.json"
            source.parent.mkdir(parents=True)
            source.write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "candidates": [
                            {
                                "id": "KCP-DEMO-001",
                                "title": "Bounded shared-memory ring",
                                "current_level": "L1",
                                "target_level": "L2",
                                "category": "transport",
                                "validated_chips": ["demo"],
                                "occurrences": 1,
                                "evidence": ["verification.md"],
                                "applicability": (
                                    "Single-producer/single-consumer channels"
                                ),
                                "non_applicability": "Multi-writer channels",
                                "counterexamples": ["Unbounded queue"],
                                "chip_specific_tokens": [],
                                "approval": {
                                    "status": "pending",
                                    "approved_by": "",
                                },
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            paths = collect_candidates(change, output)
            data = json.loads(paths[0].read_text(encoding="utf-8"))
            self.assertEqual(data["source_change"], str(change.resolve()))
            self.assertEqual(data["approval"]["status"], "pending")

    def test_l3_ready_candidate_returns_review_not_pass(self):
        candidate = {
            "id": "KCP-DEMO-002",
            "source_change": "/tmp/demo-change",
            "current_level": "L2",
            "target_level": "L3",
            "validated_chips": ["demo"],
            "occurrences": 2,
            "evidence": ["a", "b"],
            "applicability": "Bounded endpoint transport",
            "non_applicability": "Shared multi-writer bus",
            "counterexamples": ["Direct callback transport"],
            "chip_specific_tokens": [],
            "approval": {"status": "pending", "approved_by": ""},
        }
        results = evaluate_candidate(candidate)
        self.assertTrue(any(item.status == "REVIEW" for item in results))
        self.assertFalse(
            any(item.message == "promotion approved" for item in results)
        )

    def test_l3_rejects_chip_specific_tokens(self):
        candidate = {
            "id": "KCP-DEMO-003",
            "source_change": "/tmp/demo-change",
            "current_level": "L2",
            "target_level": "L3",
            "validated_chips": ["demo"],
            "occurrences": 2,
            "evidence": ["a", "b"],
            "applicability": "Transport",
            "non_applicability": "None",
            "counterexamples": ["Direct callback"],
            "chip_specific_tokens": ["NGU800", "0x12340000"],
            "approval": {"status": "pending", "approved_by": ""},
        }
        results = evaluate_candidate(candidate)
        self.assertTrue(any(item.status == "FAIL" for item in results))
