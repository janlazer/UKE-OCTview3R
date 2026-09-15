"""Lightweight manuscript consistency checks; no scientific validation implied."""

import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
paper = root / 'paper' / 'paper.md'
source = paper.read_text(encoding='utf-8')
parts = source.split('---', 2)
assert len(parts) == 3 and not parts[0].strip(), 'Missing YAML front matter'
body = re.sub(r'<!--.*?-->', '', parts[2], flags=re.S)
required = {'Summary', 'Statement of need', 'State of the field', 'Software design',
            'Research impact statement', 'AI usage disclosure', 'Acknowledgements', 'References'}
headings = set(re.findall(r'^# (.+)$', body, re.M))
assert required <= headings, f'Missing sections: {required - headings}'

bib = (paper.parent / 'paper.bib').read_text(encoding='utf-8')
entry_list = re.findall(r'^@\w+\{([^,]+),', bib, re.M)
entries = set(entry_list)
assert len(entries) == len(entry_list), 'Duplicate bibliography keys'
cited = {item.strip().lstrip('@').partition(',')[0].strip()
         for group in re.findall(r'\[@([^\]]+)\]', body)
         for item in group.split(';')}
assert cited == entries, f'Unknown: {cited - entries}; uncited: {entries - cited}'
figures = re.findall(r'!\[.*?\]\(([^)]+)\)', body, re.S)
assert figures, 'No figures found'
for figure in figures:
    assert (paper.parent / figure).is_file(), f'Missing figure: {figure}'

words = len(body.split())
assert 750 <= words <= 1750, f'Approximate word count outside JOSS guidance: {words}'
print(f'PASS: {len(required)} sections, {len(entries)} cited references, '
      f'{len(figures)} figures, approximately {words} words including captions.')
print('Author review, image permissions, research claims, and JOSS eligibility remain separate checks.')
