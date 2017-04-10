/*
* @Author: Daniel
* @Date:   2017-04-10 09:50:44
* @Last Modified by:   Daniel
* @Last Modified time: 2017-04-10 11:38:56
*/

#ifndef FEMTO_DST_READER_H
#define FEMTO_DST_READER_H

#include "TreeAnalyzer.h"

// FemtoDst Format
#include "FemtoDstFormat/BranchReader.h"
#include "FemtoDstFormat/TClonesArrayReader.h"
#include "FemtoDstFormat/FemtoEvent.h"
#include "FemtoDstFormat/FemtoTrack.h"
#include "FemtoDstFormat/FemtoMcTrack.h"
#include "FemtoDstFormat/FemtoMtdPidTraits.h"

#include "vendor/loguru.h"


// ROOT
#include "TLorentzVector.h"


class FemtoDstReader : public TreeAnalyzer
{
public:
	virtual const char* classname() const { return "FemtoDstReader"; }
	FemtoDstReader() {}
	~FemtoDstReader() {}

	virtual void initialize(){
		LOG_SCOPE_FUNCTION( INFO );
		TreeAnalyzer::initialize();

		
		_rEvent.setup( chain, "Event" );
		_rTracks.setup( chain, "Tracks" );
		_rMcTracks.setup( chain, "McTracks" );
		_rMtdPid.setup( chain, "MtdPidTraits" );
		

		mass1 = config.getFloat( "mass1", 0.105 );
		mass2 = config.getFloat( "mass2", 0.105 );

		book->cd();

	}


protected:

	class TrackContainer {
	public:
		FemtoTrack *_track;
		FemtoMcTrack *_mcTrack;
		FemtoMtdPidTraits *_mtdPid;
	};

	TrackContainer getTrackContainer( UInt_t iMcTrack ){

		TrackContainer tc;
		tc._mcTrack   = _rMcTracks.get( iMcTrack );
		
		if ( tc._mcTrack->mAssociatedIndex >= 0 )
			tc._track = _rTracks.get( tc._mcTrack->mAssociatedIndex );
		else 
			tc._track = nullptr;

		if ( nullptr != tc._track && tc._track->mMtdPidTraitsIndex >= 0 )
			tc._mtdPid = _rMtdPid.get( tc._track->mMtdPidTraitsIndex );
		else 
			tc._mtdPid = nullptr;
		return tc;
	}


	BranchReader<FemtoEvent> _rEvent;
	FemtoEvent * _event;
	TClonesArrayReader<FemtoTrack> _rTracks;
	TClonesArrayReader<FemtoMcTrack> _rMcTracks;
	TClonesArrayReader<FemtoMtdPidTraits> _rMtdPid;

	float mass1, mass2;


	virtual void histogramPair( TrackContainer &tc1, TrackContainer &tc2, string prefix = "" ){
		
		TLorentzVector lv1, lv2, lv;
		lv1.SetPtEtaPhiM( tc1._mcTrack->mPt, tc1._mcTrack->mEta, tc1._mcTrack->mPhi, mass1 );
		lv2.SetPtEtaPhiM( tc2._mcTrack->mPt, tc2._mcTrack->mEta, tc2._mcTrack->mPhi, mass2 );

		// LOG_F( INFO, "P1( %f, %f, %f )", tc1._mcTrack->mPt, tc1._mcTrack->mEta, tc1._mcTrack->mPhi );
		// LOG_F( INFO, "P2( %f, %f, %f )", tc2._mcTrack->mPt, tc2._mcTrack->mEta, tc2._mcTrack->mPhi );

		lv = lv1 + lv2;

		double m = lv.M();
		double pt = lv.Pt();

		book->fill( prefix + "mc_pT_mass", m, pt );
		if ( nullptr != tc1._track && nullptr != tc2._track ){
			book->fill( prefix + "rc_pT_mass", m, pt );

			if ( nullptr != tc1._mtdPid && nullptr != tc2._mtdPid ){
				book->fill( prefix + "mtd_pT_mass", m, pt );
			} // both MTD
		} // both RC
	}

	virtual void analyzeEvent(){
		_event = _rEvent.get();

		if ( nullptr == _event ){
			LOG_F( ERROR, "Null Event" );
		}

		vector<TrackContainer> posMuons;
		vector<TrackContainer> negMuons;

		UInt_t nMcTracks = _rMcTracks.N();
		for ( UInt_t i = 0; i < nMcTracks; i++ ){
			TrackContainer tc = getTrackContainer( i );

			if ( nullptr != tc._mcTrack && tc._mcTrack->mParentIndex < 0 ){
				if ( tc._mcTrack->mGeantPID == 5 ){
					posMuons.push_back( tc );
				} else if ( tc._mcTrack->mGeantPID == 6 ){
					negMuons.push_back( tc );
				}
			} // not nullptr, and primary MC plc
		} // loop on tracks

		// Make the pairs
		uint nPos = posMuons.size();
		uint nNeg = negMuons.size();
		for ( uint i = 0; i < nPos; i++ ){
			
			for ( uint j = 0; j < nNeg; j++ ){
				histogramPair( posMuons[ i ], negMuons[ j ], "" );
			} //loop j
		} // loop i


		for ( uint i = 0; i < nPos; i++ ){
			for ( uint j = i; j < nPos; j++ ){
				if ( i==j ) continue;
				histogramPair( posMuons[i], posMuons[j], "lsp_" );
			}
		}

		for ( uint i = 0; i < nNeg; i++ ){
			for ( uint j = i; j < nNeg; j++ ){
				if ( i==j ) continue;
				histogramPair( negMuons[i], negMuons[j], "lsn_" );
			}
		}


		// LOG_F( INFO, "#pos=%lu, #neg=%lu", posMuons.size(), negMuons.size() );

	}

	
};


#endif