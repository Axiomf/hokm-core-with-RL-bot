import json
import os
from pathlib import Path
import subprocess
import sys
import pytest
from hokm import Engine, Phase, PlayCard, ChooseTrump, Suit, encode, decode
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from simulate import run

def observation(o):
    names = "player team hakem dealer trump actor leader phase hand hand_sizes tricks score match_winner legal_mask".split()
    result = {n: getattr(o, n) for n in names}
    result.update(trick=[(c.player,c.card) for c in o.trick], history=[(c.player,c.card) for c in o.history],
                  result=(o.result.winner,o.result.points,o.result.tricks,o.result.hakem))
    return result

def test_encoding_and_invalid_atomicity():
    for a in range(56):
        assert encode(decode(a)) == a
    e=Engine(10,2)
    before=observation(e.observe(2))
    for a in [-1,0,56,999]:
        with pytest.raises(ValueError): e.apply_id(a)
        assert observation(e.observe(2))==before
    with pytest.raises(ValueError): e.start_next_round()
    e.apply(ChooseTrump(Suit.CLUBS))
    e.apply(PlayCard(e.legal_actions()[0]))
    e.check_invariants()

def test_hidden_isolation_and_snapshots():
    d=list(range(52)); hidden=d.copy(); hidden[5],hidden[10]=hidden[10],hidden[5]
    a,b=Engine(),Engine(); a.load_deal(d,0); b.load_deal(hidden,0)
    for phase in range(2):
        assert observation(a.observe(0))==observation(b.observe(0))
        assert a.legal_action_mask()==b.legal_action_mask()
        if phase==0: a.apply_id(55); b.apply_id(55)
    o=a.observe(0); hand=o.hand; hand.clear()
    assert len(a.observe(0).hand)==13
    with pytest.raises(AttributeError): o.actor=2
    assert not any(a.observe(1).legal_mask)
    for attr in ["deck", "seed", "rng", "hands", "state"]:
        assert not hasattr(o,attr)
    clone=a.clone(); clone.apply_id(clone.legal_actions()[0])
    assert a.current_actor==0 and clone.current_actor==1

def test_fixture_replay_and_validation():
    a=Engine(500); b=Engine(999); b.load_deal(a.debug_state().deck, a.observe(0).hakem)
    while a.phase in (Phase.CHOOSE_TRUMP,Phase.PLAY):
        action=a.legal_actions()[-1]; a.apply_id(action); b.apply_id(action)
        assert observation(a.observe(0))==observation(b.observe(0))
        a.check_invariants(); b.check_invariants()
    before=observation(a.observe(0))
    for deck,h,score in [([0]*52,0,[0,0]),(list(range(52)),4,[0,0]),(list(range(52)),0,[7,0])]:
        with pytest.raises(ValueError): a.load_deal(deck,h,score)
        assert observation(a.observe(0))==before

def test_native_binding_agreement():
    native=Path(os.environ.get("HOKM_NATIVE_SIM", "build/hokm_sim"))
    if not native.exists():
        pytest.fail("Build hokm_sim first or set HOKM_NATIVE_SIM to its absolute path")
    output=subprocess.check_output([str(native.resolve()),"10","123","trace"],text=True).splitlines()
    expected=[int(x) for x in output[0].split()]
    stats,actual=run(10,123,"trace")
    assert actual==expected
    native_stats=json.loads(output[1])
    for key in ["steps","rounds","checksum"]: assert stats[key]==native_stats[key]

@pytest.mark.parametrize("seed",range(10))
def test_random_matches(seed):
    e=Engine(seed)
    while e.phase!=Phase.MATCH_OVER:
        e.check_invariants()
        if e.phase==Phase.ROUND_OVER: e.start_next_round()
        else: e.apply_id(e.legal_actions()[seed%len(e.legal_actions())])
    e.check_invariants()
    assert e.observe(0).score[e.match_winner]>=7
