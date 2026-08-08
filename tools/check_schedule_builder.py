#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "src/web_ui_v2.h"


def fail(message: str) -> None:
    print(f"schedule-builder check: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    text = UI.read_text(encoding="utf-8")

    required = [
        'id="schedule-builder"',
        'id="builder-sunrise"',
        'id="builder-sunset"',
        'data-builder-source="solar"',
        'data-builder-source="custom"',
        'data-builder-mode="interval"',
        'data-builder-mode="count"',
        'id="builder-start-offset"',
        'id="builder-end-offset"',
        'id="builder-interval"',
        'id="builder-count"',
        'id="builder-duration"',
        'id="builder-daybar"',
        'id="builder-cycle-layer"',
        'id="builder-times"',
        'id="btn-builder-apply"',
        'function builderPlan()',
        'function renderScheduleBuilder()',
        'function applyBuilderSchedule()',
        'function paintSolarTrack(',
        'lastStatus?.sunrise',
        'lastStatus?.sunset',
        'MAX_SLOTS',
        'Перенести в расписание',
        'В конструктор расписания',
        'markScheduleDirty(true)',
        'btn-save-schedule',
    ]
    missing = [token for token in required if token not in text]
    if missing:
        fail("missing contract tokens: " + ", ".join(missing))

    # The old hydraulics-only generator bypassed the dedicated schedule builder.
    if "for(let t=c.start;t<c.end&&slots.length<MAX_SLOTS;t+=c.interval)" in text:
        fail("legacy direct hydraulics schedule generator returned")

    # Solar building is a preview/draft operation; it must not POST the schedule
    # from the builder button itself. Persistence remains the existing explicit
    # Save action on the schedule editor.
    apply_start = text.find("function applyBuilderSchedule()")
    if apply_start < 0:
        fail("applyBuilderSchedule function not found")
    apply_end = text.find("function ", apply_start + 10)
    apply_body = text[apply_start: apply_end if apply_end > 0 else len(text)]
    if "api('/api/schedule'" in apply_body or 'api("/api/schedule"' in apply_body:
        fail("builder writes schedule directly instead of creating a draft")

    print("schedule-builder check: OK (solar/custom, interval/count, preview -> draft -> explicit save)")


if __name__ == "__main__":
    main()
